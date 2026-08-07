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

//! Raw FFI declarations for libCEC's C API.
//!
//! This module mirrors [`include/cecc.h`] and the protocol types in
//! [`include/cectypes.h`] by hand, the same way `src/dotnetlib/cs/CecInterop.cs`
//! mirrors them for .NET. Nothing is generated: keeping the declarations in
//! source form is what lets this crate build with no dependencies at all, so
//! `cargo build --offline` works and neither cmake nor the Debian package needs
//! a vendored registry or a `libclang` at build time.
//!
//! The safety net for that choice is `tests/layout.rs`, which asserts every size
//! and field offset against numbers taken from a C compiler reading the real
//! headers. Change a struct here and it fails.
//!
//! Two conventions carried over from `CecInterop.cs`:
//!
//! * **C enums are `c_int`.** Every enum in `cectypes.h` fits in a signed 32-bit
//!   int (the largest value is `CEC_VENDOR_HARMAN_KARDON`, `0x9C645E`), so a C
//!   compiler gives them all `int` as the underlying type. Using `c_int` here
//!   rather than a Rust `enum` also means a value libCEC invents tomorrow cannot
//!   turn into undefined behaviour: it arrives as an unrecognised integer and the
//!   safe layer decides what to do with it.
//! * **Structs are plain data.** No lifetimes, no `NonNull`, so an all-zero bit
//!   pattern is valid for every one of them and [`Default`] can be `zeroed()`.
//!
//! [`include/cecc.h`]: https://github.com/Pulse-Eight/libcec/blob/master/include/cecc.h
//! [`include/cectypes.h`]: https://github.com/Pulse-Eight/libcec/blob/master/include/cectypes.h

// C names, kept verbatim so a declaration here can be diffed against the header
// it came from - `cec_logical_address` and `iPhysicalAddress`, not the shapes
// rustc would prefer. The safe layer above is where Rust naming starts.
#![allow(non_camel_case_types, non_snake_case)]
// This module is a mechanical mirror of two C headers. The types and the
// interesting functions carry documentation; requiring it on all ~90 entry
// points would only produce paraphrases of their own names.
#![allow(missing_docs)]

use std::os::raw::{c_char, c_int, c_uint, c_void};

// ---------------------------------------------------------------------------
// sizes
// ---------------------------------------------------------------------------

/// `CEC_MAX_DATA_PACKET_SIZE` - the parameter bytes a single command can carry.
pub const CEC_MAX_DATA_PACKET_SIZE: usize = 16 * 4;

/// `LIBCEC_OSD_NAME_SIZE` - the width of [`libcec_configuration::strDeviceName`],
/// name plus terminator. This is 15 from libCEC 5 onwards, and 13 before it.
pub const LIBCEC_OSD_NAME_SIZE: usize = 15;

/// `cec_osd_name` - the buffer [`libcec_get_device_osd_name`] fills in.
///
/// Note this is *not* [`LIBCEC_OSD_NAME_SIZE`]: the two have disagreed since
/// libCEC 5 widened the configuration field and left the callee-filled buffer
/// alone. Pass 14 bytes here, whatever the configuration field holds.
pub const CEC_OSD_NAME_SIZE: usize = 14;

/// `cec_menu_language` - a 3-character ISO 639-2 code plus terminator.
pub const CEC_MENU_LANGUAGE_SIZE: usize = 4;

/// The device types one client can announce.
pub const CEC_DEVICE_TYPE_LIST_SIZE: usize = 5;

/// The number of CEC logical addresses (0..=15).
pub const CEC_LOGICAL_ADDRESS_COUNT: usize = 16;

/// Width of the two path buffers in [`cec_adapter_descriptor`].
pub const CEC_ADAPTER_PATH_SIZE: usize = 1024;

/// Width of [`cec_adapter_descriptor::strDeviceName`].
pub const CEC_ADAPTER_NAME_SIZE: usize = 64;

// ---------------------------------------------------------------------------
// enum aliases
//
// One alias per C enum, so a declaration below reads like the header it came
// from and the safe layer can convert at a single, obvious boundary.
// ---------------------------------------------------------------------------

pub type cec_abort_reason = c_int;
pub type cec_adapter_type = c_int;
pub type cec_audio_status = c_int;
pub type cec_bus_device_status = c_int;
pub type cec_deck_control_mode = c_int;
pub type cec_deck_info = c_int;
pub type cec_device_type = c_int;
pub type cec_display_control = c_int;
pub type cec_log_level = c_int;
pub type cec_logical_address = c_int;
pub type cec_menu_state = c_int;
pub type cec_opcode = c_int;
pub type cec_play_mode = c_int;
pub type cec_power_status = c_int;
pub type cec_system_audio_status = c_int;
pub type cec_user_control_code = c_int;
pub type cec_vendor_id = c_int;
pub type cec_version = c_int;
pub type libcec_alert = c_int;
pub type libcec_parameter_type = c_int;

/// An opaque libCEC connection - a `CEC::ICECAdapter*` on the other side.
pub type libcec_connection_t = *mut c_void;

// ---------------------------------------------------------------------------
// structs
// ---------------------------------------------------------------------------

/// Give a plain-data struct a zeroed [`Default`].
///
/// Sound for every type in this module: they hold integers, `c_char` arrays,
/// raw pointers and nullable function pointers, and all-zero is a valid bit
/// pattern for each. It exists because `[T; N]` only implements `Default` up to
/// N = 32, and several of these carry 64- and 1024-byte buffers.
macro_rules! zeroed_default {
    ($($t:ty),* $(,)?) => {$(
        impl Default for $t {
            fn default() -> Self {
                // SAFETY: plain data, no niches, no validity invariants.
                unsafe { ::std::mem::zeroed() }
            }
        }
    )*};
}

/// The parameter bytes attached to a command.
///
/// `data` is a fixed 64-byte buffer of which only the first `size` bytes are
/// meaningful; the rest is whatever was there before.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_datapacket {
    pub data: [u8; CEC_MAX_DATA_PACKET_SIZE],
    pub size: u8,
}

/// One CEC message.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_command {
    pub initiator: cec_logical_address,
    pub destination: cec_logical_address,
    /// 1 when the ACK bit is set.
    pub ack: i8,
    /// 1 when the EOM bit is set.
    pub eom: i8,
    pub opcode: cec_opcode,
    pub parameters: cec_datapacket,
    /// 0 for a POLL message, which carries no opcode.
    pub opcode_set: i8,
    /// Transmit timeout in ms.
    pub transmit_timeout: i32,
}

/// A log line from libCEC.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_log_message {
    /// Borrowed for the duration of the callback only - copy it, do not keep it.
    pub message: *const c_char,
    pub level: cec_log_level,
    /// Timestamp in ms since libCEC started.
    pub time: i64,
}

/// A keypress forwarded from the bus.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_keypress {
    pub keycode: cec_user_control_code,
    /// 0 for a press, the held duration in ms for a release.
    pub duration: c_uint,
}

/// The pre-1.8.2 adapter description, still returned by [`libcec_find_adapters`].
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_adapter {
    pub path: [c_char; CEC_ADAPTER_PATH_SIZE],
    pub comm: [c_char; CEC_ADAPTER_PATH_SIZE],
}

/// A detected adapter, as returned by [`libcec_detect_adapters`].
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_adapter_descriptor {
    /// Stable location: USB-tree path on Linux, device instance id on Windows.
    /// Unlike `strComName` this does not move when enumeration order changes.
    pub strComPath: [c_char; CEC_ADAPTER_PATH_SIZE],
    /// The port to open, e.g. `/dev/ttyACM0` or `COM3`.
    pub strComName: [c_char; CEC_ADAPTER_PATH_SIZE],
    pub iVendorId: u16,
    pub iProductId: u16,
    pub iFirmwareVersion: u16,
    pub iPhysicalAddress: u16,
    pub iFirmwareBuildDate: u32,
    pub adapterType: cec_adapter_type,
    /// Human-readable display name, e.g. "HDMI 1".
    pub strDeviceName: [c_char; CEC_ADAPTER_NAME_SIZE],
}

/// The device types a client announces on the bus.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_device_type_list {
    pub types: [cec_device_type; CEC_DEVICE_TYPE_LIST_SIZE],
}

/// A set of logical addresses.
///
/// `addresses` is indexed *by* logical address and holds a flag, not a list:
/// `addresses[3] != 0` means address 3 is a member.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_logical_addresses {
    pub primary: cec_logical_address,
    pub addresses: [c_int; CEC_LOGICAL_ADDRESS_COUNT],
}

/// Extra data attached to an alert.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct libcec_parameter {
    pub paramType: libcec_parameter_type,
    pub paramData: *mut c_void,
}

/// Adapter frame counters, filled in by [`libcec_get_stats`].
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct cec_adapter_stats {
    pub tx_ack: c_uint,
    pub tx_nack: c_uint,
    pub tx_error: c_uint,
    pub rx_total: c_uint,
    pub rx_error: c_uint,
}

// The callback signatures. libCEC invokes all of these from its own worker
// thread, never from the caller's, and `CEC_CDECL` is `__cdecl` on 32-bit
// Windows - which is what `extern "C"` already means there.
pub type cec_log_message_cb = extern "C" fn(cbparam: *mut c_void, message: *const cec_log_message);
pub type cec_keypress_cb = extern "C" fn(cbparam: *mut c_void, key: *const cec_keypress);
pub type cec_command_cb = extern "C" fn(cbparam: *mut c_void, command: *const cec_command);
pub type cec_configuration_cb =
    extern "C" fn(cbparam: *mut c_void, configuration: *const libcec_configuration);
pub type cec_alert_cb =
    extern "C" fn(cbparam: *mut c_void, alert: libcec_alert, param: libcec_parameter);
pub type cec_menu_state_cb = extern "C" fn(cbparam: *mut c_void, state: cec_menu_state) -> c_int;
pub type cec_source_activated_cb =
    extern "C" fn(cbparam: *mut c_void, logical_address: cec_logical_address, activated: u8);
pub type cec_command_handler_cb =
    extern "C" fn(cbparam: *mut c_void, command: *const cec_command) -> c_int;

/// The callback table handed to libCEC through [`libcec_configuration::callbacks`].
///
/// libCEC keeps the *pointer*, not a copy, so whatever holds this must not move
/// or drop while a connection is open. Leave a field `None` and libCEC skips
/// that notification entirely.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct ICECCallbacks {
    pub logMessage: Option<cec_log_message_cb>,
    pub keyPress: Option<cec_keypress_cb>,
    pub commandReceived: Option<cec_command_cb>,
    pub configurationChanged: Option<cec_configuration_cb>,
    pub alert: Option<cec_alert_cb>,
    /// Returns 1 to let libCEC apply the new menu state, 0 to keep the device
    /// activated. libCEC gives up waiting after 1000ms.
    pub menuStateChanged: Option<cec_menu_state_cb>,
    pub sourceActivated: Option<cec_source_activated_cb>,
    /// Returns 1 when the client has handled the command and libCEC should not
    /// act on it itself. Same 1000ms budget as `menuStateChanged`.
    pub commandHandler: Option<cec_command_handler_cb>,
}

/// A client configuration - the argument to [`libcec_initialise`].
///
/// **This layout is major-version specific.** libCEC appends fields on every
/// major bump, so a struct built here is only valid against a libCEC 8. `build.rs`
/// refuses to build against a different major for that reason.
///
/// Fields marked read-only are filled in by libCEC and ignored on the way in.
#[repr(C)]
#[derive(Copy, Clone, Debug)]
pub struct libcec_configuration {
    /// The version of the client connecting. [`libcec_clear_configuration`] sets
    /// it; do not write it by hand.
    pub clientVersion: u32,
    /// The OSD name to announce, NUL-terminated.
    pub strDeviceName: [c_char; LIBCEC_OSD_NAME_SIZE],
    pub deviceTypes: cec_device_type_list,
    /// (read-only) 1 when libCEC autodetected the physical address.
    pub bAutodetectAddress: u8,
    /// The adapter's physical address, or 0 to derive it from
    /// `baseDevice` + `iHDMIPort`.
    pub iPhysicalAddress: u16,
    /// The device the adapter is plugged into. Only used when
    /// `iPhysicalAddress` is 0 or autodetection is unavailable.
    pub baseDevice: cec_logical_address,
    /// The HDMI port the adapter is plugged into, same condition as `baseDevice`.
    pub iHDMIPort: u8,
    /// Override the TV's vendor id, or `CEC_VENDOR_UNKNOWN` to autodetect.
    pub tvVendor: u32,
    /// Devices to wake on start-up and on a bare `PowerOnDevices()`.
    pub wakeDevices: cec_logical_addresses,
    /// Devices to put in standby on a bare `StandbyDevices()`.
    pub powerOffDevices: cec_logical_addresses,
    /// (read-only) the version of libCEC on the other side.
    pub serverVersion: u32,
    /// Take the settings from the adapter's EEPROM instead of this struct.
    pub bGetSettingsFromROM: u8,
    /// Become the active source when the connection opens.
    pub bActivateSource: u8,
    /// Put this host in standby when the TV switches off.
    pub bPowerOffOnStandby: u8,
    /// Passed back to every callback. The safe layer points this at its own state.
    pub callbackParam: *mut c_void,
    /// Borrowed, not copied - see [`ICECCallbacks`].
    pub callbacks: *mut ICECCallbacks,
    /// (read-only) the addresses this client currently holds.
    pub logicalAddresses: cec_logical_addresses,
    /// (read-only) the adapter's firmware version.
    pub iFirmwareVersion: u16,
    /// 3-character ISO 639-2 menu language, *not* NUL-terminated.
    pub strDeviceLanguage: [c_char; 3],
    /// (read-only) firmware build date, seconds since epoch, or 0 if unknown.
    pub iFirmwareBuildDate: u32,
    /// Watch the bus without claiming a logical address.
    pub bMonitorOnly: u8,
    /// The CEC version to advertise. Defaults to 1.4.
    pub cecVersion: cec_version,
    /// (read-only) which backend is in use.
    pub adapterType: cec_adapter_type,
    /// The key that starts a combo, or `CEC_USER_CONTROL_CODE_UNKNOWN` to disable.
    pub comboKey: cec_user_control_code,
    /// How long a combo key waits before being delivered as a normal press.
    pub iComboKeyTimeoutMs: u32,
    /// Auto-repeat rate for held buttons; 0 defers to the CEC device.
    pub iButtonRepeatRateMs: u32,
    /// How long after the last update a button counts as released.
    pub iButtonReleaseDelayMs: u32,
    /// Suppress a repeated press/release of the same key inside this window.
    /// 0 (the default) forwards every press.
    pub iDoubleTapTimeoutMs: u32,
    /// Wake an AVR automatically when the source is activated.
    pub bAutoWakeAVR: u8,
    /// Wake the TV when the adapter is powered. Needs an EEPROM save to persist.
    /// Added in libCEC 5.0.0 / firmware v9.
    pub bAutoPowerOn: u8,
    /// Let the adapter stay active on the bus while the host is down (1, the
    /// default) or keep it silent when unattended (0). Added in 8.0.0.
    pub bAutonomousMode: u8,
    /// Delay before a held button starts repeating. Added in 8.0.0.
    pub iButtonRepeatDelayMs: u32,
    /// The vendor id to announce for this device. Added in 8.0.0.
    pub iDeviceVendorId: u32,
}

zeroed_default!(
    cec_datapacket,
    cec_command,
    cec_log_message,
    cec_keypress,
    cec_adapter,
    cec_adapter_descriptor,
    cec_device_type_list,
    cec_logical_addresses,
    libcec_parameter,
    cec_adapter_stats,
    ICECCallbacks,
    libcec_configuration,
);

// ---------------------------------------------------------------------------
// functions
//
// Declared against libCEC 8, so the `CEC_LIB_VERSION_MAJOR >= 5` half of every
// conditional pair in cecc.h is the one that appears here: libcec_set_callbacks
// rather than libcec_enable_callbacks, libcec_can_save_configuration rather than
// libcec_can_persist_configuration.
//
// Return conventions: `c_int` is a C boolean unless noted (1 success, 0 failure),
// and the `_to_string` helpers write into a caller-supplied buffer.
// ---------------------------------------------------------------------------

extern "C" {
    // -- lifecycle ----------------------------------------------------------

    /// Returns a connection, or null when initialisation fails.
    pub fn libcec_initialise(configuration: *mut libcec_configuration) -> libcec_connection_t;
    /// Stops the worker thread and frees the connection. No callback fires after
    /// this returns, which is what makes it safe to drop callback state next.
    pub fn libcec_destroy(connection: libcec_connection_t);
    /// `strPort` may be null to open the first adapter found.
    pub fn libcec_open(
        connection: libcec_connection_t,
        strPort: *const c_char,
        iTimeout: u32,
    ) -> c_int;
    pub fn libcec_close(connection: libcec_connection_t);
    /// Resets `configuration` to libCEC's defaults. Only writes, so it is safe to
    /// call on uninitialised memory - which is how a zeroed struct becomes valid.
    pub fn libcec_clear_configuration(configuration: *mut libcec_configuration);
    pub fn libcec_set_callbacks(
        connection: libcec_connection_t,
        callbacks: *mut ICECCallbacks,
        cbParam: *mut c_void,
    ) -> c_int;
    pub fn libcec_disable_callbacks(connection: libcec_connection_t) -> c_int;

    // -- adapters -----------------------------------------------------------

    /// Superseded by [`libcec_detect_adapters`]; kept because it is still exported.
    pub fn libcec_find_adapters(
        connection: libcec_connection_t,
        deviceList: *mut cec_adapter,
        iBufSize: u8,
        strDevicePath: *const c_char,
    ) -> i8;
    /// Fills up to `iBufSize` entries and returns how many, or -1 on failure.
    pub fn libcec_detect_adapters(
        connection: libcec_connection_t,
        deviceList: *mut cec_adapter_descriptor,
        iBufSize: u8,
        strDevicePath: *const c_char,
        bQuickScan: c_int,
    ) -> i8;
    pub fn libcec_ping_adapters(connection: libcec_connection_t) -> c_int;
    pub fn libcec_start_bootloader(connection: libcec_connection_t) -> c_int;
    pub fn libcec_get_adapter_vendor_id(connection: libcec_connection_t) -> u16;
    pub fn libcec_get_adapter_product_id(connection: libcec_connection_t) -> u16;
    pub fn libcec_get_stats(
        connection: libcec_connection_t,
        stats: *mut cec_adapter_stats,
    ) -> c_int;

    // -- power / source -----------------------------------------------------

    pub fn libcec_power_on_devices(
        connection: libcec_connection_t,
        address: cec_logical_address,
    ) -> c_int;
    pub fn libcec_standby_devices(
        connection: libcec_connection_t,
        address: cec_logical_address,
    ) -> c_int;
    pub fn libcec_set_active_source(
        connection: libcec_connection_t,
        type_: cec_device_type,
    ) -> c_int;
    pub fn libcec_set_inactive_view(connection: libcec_connection_t) -> c_int;
    pub fn libcec_get_active_source(connection: libcec_connection_t) -> cec_logical_address;
    pub fn libcec_is_active_source(
        connection: libcec_connection_t,
        iAddress: cec_logical_address,
    ) -> c_int;
    pub fn libcec_is_libcec_active_source(connection: libcec_connection_t) -> c_int;
    pub fn libcec_set_stream_path_logical(
        connection: libcec_connection_t,
        iAddress: cec_logical_address,
    ) -> c_int;
    pub fn libcec_set_stream_path_physical(
        connection: libcec_connection_t,
        iPhysicalAddress: u16,
    ) -> c_int;

    // -- transmit / input ---------------------------------------------------

    pub fn libcec_transmit(connection: libcec_connection_t, data: *const cec_command) -> c_int;
    pub fn libcec_send_keypress(
        connection: libcec_connection_t,
        iDestination: cec_logical_address,
        key: cec_user_control_code,
        bWait: c_int,
    ) -> c_int;
    pub fn libcec_send_key_release(
        connection: libcec_connection_t,
        iDestination: cec_logical_address,
        bWait: c_int,
    ) -> c_int;
    pub fn libcec_send_play(
        connection: libcec_connection_t,
        iDestination: cec_logical_address,
        mode: cec_play_mode,
    ) -> c_int;
    pub fn libcec_set_osd_string(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
        duration: cec_display_control,
        strMessage: *const c_char,
    ) -> c_int;

    // -- audio --------------------------------------------------------------

    /// Returns the new audio status, not a boolean.
    pub fn libcec_volume_up(connection: libcec_connection_t, bSendRelease: c_int) -> c_int;
    /// Returns the new audio status, not a boolean.
    pub fn libcec_volume_down(connection: libcec_connection_t, bSendRelease: c_int) -> c_int;
    /// Returns the new audio status, not a boolean.
    pub fn libcec_mute_audio(connection: libcec_connection_t, bSendRelease: c_int) -> c_int;
    pub fn libcec_audio_toggle_mute(connection: libcec_connection_t) -> u8;
    pub fn libcec_audio_mute(connection: libcec_connection_t) -> u8;
    pub fn libcec_audio_unmute(connection: libcec_connection_t) -> u8;
    pub fn libcec_audio_get_status(connection: libcec_connection_t) -> u8;
    pub fn libcec_system_audio_mode(connection: libcec_connection_t, bEnable: c_int) -> c_int;
    pub fn libcec_system_audio_mode_get_status(connection: libcec_connection_t) -> u8;

    // -- bus queries --------------------------------------------------------

    pub fn libcec_poll_device(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> c_int;
    pub fn libcec_get_active_devices(connection: libcec_connection_t) -> cec_logical_addresses;
    pub fn libcec_is_active_device(
        connection: libcec_connection_t,
        address: cec_logical_address,
    ) -> c_int;
    pub fn libcec_is_active_device_type(
        connection: libcec_connection_t,
        type_: cec_device_type,
    ) -> c_int;
    pub fn libcec_get_device_cec_version(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> cec_version;
    /// `language` must point at [`CEC_MENU_LANGUAGE_SIZE`] bytes.
    pub fn libcec_get_device_menu_language(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
        language: *mut c_char,
    ) -> c_int;
    pub fn libcec_get_device_vendor_id(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> u32;
    pub fn libcec_get_device_physical_address(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> u16;
    pub fn libcec_get_device_power_status(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> cec_power_status;
    /// `name` must point at [`CEC_OSD_NAME_SIZE`] bytes.
    pub fn libcec_get_device_osd_name(
        connection: libcec_connection_t,
        iAddress: cec_logical_address,
        name: *mut c_char,
    ) -> c_int;
    pub fn libcec_get_logical_addresses(connection: libcec_connection_t) -> cec_logical_addresses;
    pub fn libcec_rescan_devices(connection: libcec_connection_t);

    // -- configuration ------------------------------------------------------

    pub fn libcec_get_current_configuration(
        connection: libcec_connection_t,
        configuration: *mut libcec_configuration,
    ) -> c_int;
    pub fn libcec_set_configuration(
        connection: libcec_connection_t,
        configuration: *const libcec_configuration,
    ) -> c_int;
    pub fn libcec_can_save_configuration(connection: libcec_connection_t) -> c_int;
    pub fn libcec_set_logical_address(
        connection: libcec_connection_t,
        iLogicalAddress: cec_logical_address,
    ) -> c_int;
    pub fn libcec_set_physical_address(
        connection: libcec_connection_t,
        iPhysicalAddress: u16,
    ) -> c_int;
    pub fn libcec_set_hdmi_port(
        connection: libcec_connection_t,
        baseDevice: cec_logical_address,
        iPort: u8,
    ) -> c_int;
    pub fn libcec_set_deck_control_mode(
        connection: libcec_connection_t,
        mode: cec_deck_control_mode,
        bSendUpdate: c_int,
    ) -> c_int;
    pub fn libcec_set_deck_info(
        connection: libcec_connection_t,
        info: cec_deck_info,
        bSendUpdate: c_int,
    ) -> c_int;
    pub fn libcec_set_menu_state(
        connection: libcec_connection_t,
        state: cec_menu_state,
        bSendUpdate: c_int,
    ) -> c_int;
    pub fn libcec_switch_monitoring(connection: libcec_connection_t, bEnable: c_int) -> c_int;

    // -- misc ---------------------------------------------------------------

    pub fn libcec_get_device_information(
        connection: libcec_connection_t,
        strPort: *const c_char,
        config: *mut libcec_configuration,
        iTimeoutMs: u32,
    ) -> c_int;
    /// Statically allocated inside libCEC; borrow it, do not free it.
    pub fn libcec_get_lib_info(connection: libcec_connection_t) -> *const c_char;
    pub fn libcec_init_video_standalone(connection: libcec_connection_t);

    // -- enum formatting ----------------------------------------------------
    //
    // These write a NUL-terminated string into `buf`, capped at `bufsize`. Using
    // them instead of a Rust-side table is deliberate: the strings then cannot
    // drift from libCEC's own.

    pub fn libcec_menu_state_to_string(state: cec_menu_state, buf: *mut c_char, bufsize: usize);
    pub fn libcec_cec_version_to_string(version: cec_version, buf: *mut c_char, bufsize: usize);
    pub fn libcec_power_status_to_string(
        status: cec_power_status,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_logical_address_to_string(
        address: cec_logical_address,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_deck_control_mode_to_string(
        mode: cec_deck_control_mode,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_deck_status_to_string(status: cec_deck_info, buf: *mut c_char, bufsize: usize);
    pub fn libcec_opcode_to_string(opcode: cec_opcode, buf: *mut c_char, bufsize: usize);
    pub fn libcec_system_audio_status_to_string(
        mode: cec_system_audio_status,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_audio_status_to_string(
        status: cec_audio_status,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_vendor_id_to_string(vendor: cec_vendor_id, buf: *mut c_char, bufsize: usize);
    pub fn libcec_user_control_key_to_string(
        key: cec_user_control_code,
        buf: *mut c_char,
        bufsize: usize,
    );
    pub fn libcec_adapter_type_to_string(type_: cec_adapter_type, buf: *mut c_char, bufsize: usize);
    pub fn libcec_version_to_string(version: u32, buf: *mut c_char, bufsize: usize);
}
