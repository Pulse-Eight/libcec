// This file is part of the libCEC(R) library.
//
// libCEC(R) is Copyright (C) 2011-2026 Pulse-Eight Limited.  All rights reserved.
// libCEC(R) is an original work, containing original code.
//
// libCEC(R) is a trademark of Pulse-Eight Limited.
//
// This program is dual-licensed; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//
// Alternatively, you can license this library under a commercial license,
// please contact Pulse-Eight Licensing for more information.
//
// For more information contact:
// Pulse-Eight Licensing       <license@pulse-eight.com>
//     http://www.pulse-eight.com/
//     http://www.pulse-eight.net/

//! Owned Rust views of the protocol structs.
//!
//! Everything libCEC hands to a callback is borrowed for the duration of that
//! callback only, and several fields are fixed-width C buffers. These types own
//! their data, so a value can outlive the callback that produced it - which is
//! what makes [`CecEvent`](crate::CecEvent) able to travel down a channel.

use std::fmt;
use std::time::Duration;

use crate::enums::{
    AdapterType, CecVersion, DeviceType, LogLevel, LogicalAddress, Opcode, UserControlCode,
};
use crate::error::{Error, Result};
use crate::ffi;
use crate::util::{read_fixed, read_ptr, write_fixed};

/// One CEC message.
///
/// Build one to send with [`Command::new`], or receive one through
/// [`CecCallbacks::command_received`](crate::CecCallbacks::command_received).
///
/// ```
/// use libcec::{Command, enums::{LogicalAddress, Opcode}};
///
/// // "TV, switch to the input I am plugged into"
/// let command = Command::new(LogicalAddress::Tv, Opcode::ImageViewOn);
/// assert_eq!(command.destination, LogicalAddress::Tv);
/// ```
#[derive(Clone, PartialEq, Eq, Debug)]
pub struct Command {
    /// Who sent it. Leave as [`LogicalAddress::Unknown`] when sending and libCEC
    /// fills in the address this client holds.
    pub initiator: LogicalAddress,
    /// Who it is for.
    pub destination: LogicalAddress,
    /// True when the ACK bit was set. Meaningless on a message being sent.
    pub ack: bool,
    /// True when this is the end of the message.
    pub eom: bool,
    /// What the message asks for.
    pub opcode: Opcode,
    /// The bytes after the opcode. At most
    /// [`CEC_MAX_DATA_PACKET_SIZE`](crate::ffi::CEC_MAX_DATA_PACKET_SIZE).
    pub parameters: Vec<u8>,
    /// False for a POLL - a message with a destination and nothing else, used
    /// to find out whether anyone is listening at that address.
    pub opcode_set: bool,
    /// How long libCEC waits for this message to go out.
    pub transmit_timeout: Duration,
}

impl Command {
    /// A message with no parameters.
    pub fn new(destination: LogicalAddress, opcode: Opcode) -> Self {
        Command {
            initiator: LogicalAddress::Unknown,
            destination,
            ack: false,
            eom: true,
            opcode,
            parameters: Vec::new(),
            opcode_set: true,
            transmit_timeout: Duration::from_millis(1000),
        }
    }

    /// A POLL: no opcode, just a knock at a logical address.
    pub fn poll(destination: LogicalAddress) -> Self {
        Command {
            opcode_set: false,
            ..Command::new(destination, Opcode::None)
        }
    }

    /// Attach parameter bytes.
    pub fn with_parameters(mut self, parameters: impl Into<Vec<u8>>) -> Self {
        self.parameters = parameters.into();
        self
    }

    /// Send from a specific logical address rather than letting libCEC choose.
    pub fn from_initiator(mut self, initiator: LogicalAddress) -> Self {
        self.initiator = initiator;
        self
    }

    /// Override the transmit timeout.
    pub fn with_timeout(mut self, timeout: Duration) -> Self {
        self.transmit_timeout = timeout;
        self
    }

    /// Read a command out of the raw C struct.
    ///
    /// The bridge to [`ffi`](crate::ffi), for code that reaches past the safe
    /// layer - a `commandHandler` wired up by hand, say.
    pub fn from_raw(raw: &ffi::cec_command) -> Self {
        let len = (raw.parameters.size as usize).min(ffi::CEC_MAX_DATA_PACKET_SIZE);
        Command {
            initiator: LogicalAddress::from_raw(raw.initiator),
            destination: LogicalAddress::from_raw(raw.destination),
            ack: raw.ack != 0,
            eom: raw.eom != 0,
            opcode: Opcode::from_raw(raw.opcode),
            parameters: raw.parameters.data[..len].to_vec(),
            opcode_set: raw.opcode_set != 0,
            transmit_timeout: Duration::from_millis(raw.transmit_timeout.max(0) as u64),
        }
    }

    /// Build the raw C struct this command becomes on the wire.
    ///
    /// Fails when there are more parameter bytes than a CEC message can carry.
    pub fn to_raw(&self) -> Result<ffi::cec_command> {
        if self.parameters.len() > ffi::CEC_MAX_DATA_PACKET_SIZE {
            return Err(Error::ParametersTooLong(self.parameters.len()));
        }
        let mut raw = ffi::cec_command {
            initiator: self.initiator.raw(),
            destination: self.destination.raw(),
            ack: self.ack as i8,
            eom: self.eom as i8,
            opcode: self.opcode.raw(),
            opcode_set: self.opcode_set as i8,
            // Saturate rather than wrap: a timeout so long it goes negative
            // would be read by libCEC as "give up immediately".
            transmit_timeout: self.transmit_timeout.as_millis().min(i32::MAX as u128) as i32,
            ..Default::default()
        };
        raw.parameters.data[..self.parameters.len()].copy_from_slice(&self.parameters);
        raw.parameters.size = self.parameters.len() as u8;
        Ok(raw)
    }
}

impl fmt::Display for Command {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} -> {}", self.initiator, self.destination)?;
        if !self.opcode_set {
            return f.write_str(" POLL");
        }
        write!(f, " {}", self.opcode)?;
        if !self.parameters.is_empty() {
            f.write_str(" [")?;
            for (i, byte) in self.parameters.iter().enumerate() {
                if i > 0 {
                    f.write_str(" ")?;
                }
                write!(f, "{byte:02x}")?;
            }
            f.write_str("]")?;
        }
        Ok(())
    }
}

/// A remote control key, forwarded from the bus.
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct Keypress {
    /// Which key.
    pub keycode: UserControlCode,
    /// Zero while the key is down; on release, how long it was held.
    pub duration: Duration,
}

impl Keypress {
    /// True for the press, false for the matching release.
    pub fn is_press(&self) -> bool {
        self.duration.is_zero()
    }

    pub(crate) fn from_raw(raw: &ffi::cec_keypress) -> Self {
        Keypress {
            keycode: UserControlCode::from_raw(raw.keycode),
            duration: Duration::from_millis(raw.duration as u64),
        }
    }
}

impl fmt::Display for Keypress {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.is_press() {
            write!(f, "{} pressed", self.keycode)
        } else {
            write!(f, "{} released after {:?}", self.keycode, self.duration)
        }
    }
}

/// A log line from libCEC.
#[derive(Clone, PartialEq, Eq, Debug)]
pub struct LogMessage {
    /// The text, without a trailing newline.
    pub message: String,
    /// How bad it is.
    pub level: LogLevel,
    /// Time since libCEC started.
    pub time: Duration,
}

impl LogMessage {
    /// # Safety
    ///
    /// `raw.message` must be valid for the duration of the call.
    pub(crate) unsafe fn from_raw(raw: &ffi::cec_log_message) -> Self {
        LogMessage {
            message: read_ptr(raw.message),
            level: LogLevel::from_raw(raw.level),
            time: Duration::from_millis(raw.time.max(0) as u64),
        }
    }
}

impl fmt::Display for LogMessage {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "[{:?}] {}", self.level, self.message)
    }
}

/// A CEC adapter libCEC found.
#[derive(Clone, PartialEq, Eq, Debug)]
pub struct AdapterDescriptor {
    /// The adapter's stable location - USB-tree path on Linux, device instance
    /// id on Windows. Unlike [`port`](Self::port) this does not move when
    /// enumeration order changes, so it is the one to remember.
    pub path: String,
    /// The port to hand to [`ConnectionBuilder::open`](crate::ConnectionBuilder::open),
    /// e.g. `/dev/ttyACM0` or `COM3`.
    pub port: String,
    /// A human-readable name, e.g. `"HDMI 1"`.
    pub name: String,
    /// USB vendor id, or 0 for a SoC-native backend.
    pub vendor_id: u16,
    /// USB product id, or 0 for a SoC-native backend.
    pub product_id: u16,
    /// Adapter firmware version, or 0 when it was not probed.
    pub firmware_version: u16,
    /// The adapter's physical address on the HDMI tree.
    pub physical_address: u16,
    /// Firmware build date, seconds since the epoch, or 0 when unknown.
    pub firmware_build_date: u32,
    /// Which backend drives it.
    pub adapter_type: AdapterType,
}

impl AdapterDescriptor {
    pub(crate) fn from_raw(raw: &ffi::cec_adapter_descriptor) -> Self {
        AdapterDescriptor {
            path: read_fixed(&raw.strComPath),
            port: read_fixed(&raw.strComName),
            name: read_fixed(&raw.strDeviceName),
            vendor_id: raw.iVendorId,
            product_id: raw.iProductId,
            firmware_version: raw.iFirmwareVersion,
            physical_address: raw.iPhysicalAddress,
            firmware_build_date: raw.iFirmwareBuildDate,
            adapter_type: AdapterType::from_raw(raw.adapterType),
        }
    }
}

impl fmt::Display for AdapterDescriptor {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} ({})", self.port, self.adapter_type)?;
        if !self.name.is_empty() {
            write!(f, " {}", self.name)?;
        }
        Ok(())
    }
}

/// A set of logical addresses, with the one libCEC treats as primary.
#[derive(Clone, PartialEq, Eq, Debug, Default)]
pub struct LogicalAddresses {
    /// The address libCEC answers on by default.
    pub primary: LogicalAddress,
    /// Every address in the set, primary included.
    pub addresses: Vec<LogicalAddress>,
}

impl LogicalAddresses {
    /// True when `address` is in the set.
    pub fn contains(&self, address: LogicalAddress) -> bool {
        self.addresses.contains(&address)
    }

    pub(crate) fn from_raw(raw: &ffi::cec_logical_addresses) -> Self {
        // The C struct is a flag per address, indexed by the address itself -
        // addresses[3] != 0 means "3 is a member" - not a list of addresses.
        let addresses = raw
            .addresses
            .iter()
            .enumerate()
            .filter(|(_, set)| **set != 0)
            .map(|(index, _)| LogicalAddress::from_raw(index as i32))
            .collect();
        LogicalAddresses {
            primary: LogicalAddress::from_raw(raw.primary),
            addresses,
        }
    }

    pub(crate) fn to_raw(&self) -> ffi::cec_logical_addresses {
        let mut raw = ffi::cec_logical_addresses {
            primary: self.primary.raw(),
            ..Default::default()
        };
        for address in &self.addresses {
            let index = address.raw();
            if (0..ffi::CEC_LOGICAL_ADDRESS_COUNT as i32).contains(&index) {
                raw.addresses[index as usize] = 1;
            }
        }
        raw
    }
}

/// The audio status byte an amplifier reports: a mute flag and a volume.
///
/// A newtype rather than an enum because the C `cec_audio_status` is a bitfield
/// whose named values overlap - the mute bit and the volume share the byte.
#[derive(Copy, Clone, PartialEq, Eq, Debug)]
pub struct AudioStatus(pub u8);

impl AudioStatus {
    /// `CEC_AUDIO_VOLUME_STATUS_UNKNOWN`: the device did not say.
    pub const UNKNOWN: AudioStatus = AudioStatus(0x7F);

    /// True when the device reports itself muted.
    pub fn is_muted(self) -> bool {
        self.0 & 0x80 != 0 // CEC_AUDIO_MUTE_STATUS_MASK
    }

    /// Volume 0..=100, or `None` when the device did not report one.
    pub fn volume(self) -> Option<u8> {
        let volume = self.0 & 0x7F; // CEC_AUDIO_VOLUME_STATUS_MASK
        if volume == 0x7F {
            None // CEC_AUDIO_VOLUME_STATUS_UNKNOWN
        } else {
            Some(volume.min(100)) // CEC_AUDIO_VOLUME_MAX
        }
    }
}

impl fmt::Display for AudioStatus {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.volume() {
            Some(volume) if self.is_muted() => write!(f, "muted (volume {volume})"),
            Some(volume) => write!(f, "volume {volume}"),
            None if self.is_muted() => f.write_str("muted"),
            None => f.write_str("unknown"),
        }
    }
}

/// Adapter frame counters.
#[derive(Copy, Clone, PartialEq, Eq, Debug, Default)]
pub struct AdapterStats {
    /// Frames sent and acknowledged.
    pub tx_ack: u32,
    /// Frames sent and not acknowledged.
    pub tx_nack: u32,
    /// Frames that failed to go out.
    pub tx_error: u32,
    /// Frames received.
    pub rx_total: u32,
    /// Frames received with an error.
    pub rx_error: u32,
}

impl AdapterStats {
    pub(crate) fn from_raw(raw: &ffi::cec_adapter_stats) -> Self {
        AdapterStats {
            tx_ack: raw.tx_ack,
            tx_nack: raw.tx_nack,
            tx_error: raw.tx_error,
            rx_total: raw.rx_total,
            rx_error: raw.rx_error,
        }
    }
}

/// A snapshot of what libCEC is configured to do.
///
/// Returned by [`Connection::configuration`](crate::Connection::configuration)
/// and delivered to
/// [`configuration_changed`](crate::CecCallbacks::configuration_changed). To
/// *set* configuration, use [`ConnectionBuilder`](crate::ConnectionBuilder).
#[derive(Clone, PartialEq, Eq, Debug)]
pub struct Configuration {
    /// The OSD name announced on the bus.
    pub device_name: String,
    /// The device types this client claims.
    pub device_types: Vec<DeviceType>,
    /// The physical address in use.
    pub physical_address: u16,
    /// The device the adapter is plugged into.
    pub base_device: LogicalAddress,
    /// The HDMI port it is plugged into.
    pub hdmi_port: u8,
    /// The TV's vendor id, autodetected unless overridden.
    pub tv_vendor: u32,
    /// The version of the client that opened the connection.
    pub client_version: u32,
    /// The version of libCEC answering.
    pub server_version: u32,
    /// Adapter firmware version, or 0 when unknown.
    pub firmware_version: u16,
    /// Firmware build date, seconds since the epoch, or 0 when unknown.
    pub firmware_build_date: u32,
    /// The CEC version being advertised.
    pub cec_version: CecVersion,
    /// Which backend is in use.
    pub adapter_type: AdapterType,
    /// The 3-character ISO 639-2 menu language.
    pub device_language: String,
    /// True when libCEC worked the physical address out for itself.
    pub autodetect_address: bool,
    /// True when this client becomes the active source on connect.
    pub activate_source: bool,
    /// True when watching the bus without claiming an address.
    pub monitor_only: bool,
    /// The addresses this client currently holds.
    pub logical_addresses: LogicalAddresses,
}

impl Configuration {
    pub(crate) fn from_raw(raw: &ffi::libcec_configuration) -> Self {
        // CEC_DEVICE_TYPE_RESERVED is what Clear() fills the unused slots with,
        // so anything after the first of those is padding rather than a claim.
        let device_types = raw
            .deviceTypes
            .types
            .iter()
            .take_while(|t| **t != DeviceType::Reserved.raw())
            .map(|t| DeviceType::from_raw(*t))
            .collect();

        Configuration {
            device_name: read_fixed(&raw.strDeviceName),
            device_types,
            physical_address: raw.iPhysicalAddress,
            base_device: LogicalAddress::from_raw(raw.baseDevice),
            hdmi_port: raw.iHDMIPort,
            tv_vendor: raw.tvVendor,
            client_version: raw.clientVersion,
            server_version: raw.serverVersion,
            firmware_version: raw.iFirmwareVersion,
            firmware_build_date: raw.iFirmwareBuildDate,
            cec_version: CecVersion::from_raw(raw.cecVersion),
            adapter_type: AdapterType::from_raw(raw.adapterType),
            device_language: read_fixed(&raw.strDeviceLanguage),
            autodetect_address: raw.bAutodetectAddress != 0,
            activate_source: raw.bActivateSource != 0,
            monitor_only: raw.bMonitorOnly != 0,
            logical_addresses: LogicalAddresses::from_raw(&raw.logicalAddresses),
        }
    }
}

/// Format a physical address the way HDMI writes it: `1.0.0.0`.
pub fn format_physical_address(address: u16) -> String {
    format!(
        "{}.{}.{}.{}",
        (address >> 12) & 0xF,
        (address >> 8) & 0xF,
        (address >> 4) & 0xF,
        address & 0xF
    )
}

/// Helper used by the builder; kept here beside the other conversions.
pub(crate) fn set_device_name(raw: &mut ffi::libcec_configuration, name: &str) -> Result<()> {
    write_fixed(&mut raw.strDeviceName, name, "device name")
}
