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

//! The safe layer.
//!
//! Most of this is conversion logic and runs anywhere libCEC is installed. The
//! tests that need an adapter say so and skip when there is none, so the suite
//! passes on a build machine and still means something on a bench with hardware
//! attached.
//!
//! Nothing here powers a device on or off, and the connection is opened with
//! `activate_source(false)`, so running the suite cannot switch somebody's
//! television over to this machine.

use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use libcec::callbacks::channel;
use libcec::enums::{
    AdapterType, CecVersion, DeviceType, LogLevel, LogicalAddress, Opcode, UserControlCode,
    VendorId,
};
use libcec::{AudioStatus, CecCallbacks, Command, Connection, ConnectionBuilder, Error, Keypress};

// ---------------------------------------------------------------------------
// conversions - no hardware needed
// ---------------------------------------------------------------------------

#[test]
fn enums_round_trip_through_their_raw_values() {
    for value in [
        LogicalAddress::Tv,
        LogicalAddress::PlaybackDevice1,
        LogicalAddress::Unregistered,
        LogicalAddress::Unknown,
    ] {
        assert_eq!(LogicalAddress::from_raw(value.raw()), value);
    }

    // The names the .NET binding uses, at the values CEC assigns them.
    assert_eq!(LogicalAddress::Tv.raw(), 0);
    assert_eq!(LogicalAddress::RecordingDevice1.raw(), 1);
    assert_eq!(LogicalAddress::AudioSystem.raw(), 5);
    assert_eq!(LogicalAddress::Unknown.raw(), -1);

    // BROADCAST and UNREGISTERED are both 15 in CEC; the alias keeps both names
    // spelled out without asking Rust for two variants at one discriminant.
    assert_eq!(LogicalAddress::BROADCAST, LogicalAddress::Unregistered);
    assert_eq!(LogicalAddress::BROADCAST.raw(), 15);

    assert_eq!(VendorId::PulseEight.raw(), 0x001582);
    assert_eq!(DeviceType::RecordingDevice.raw(), 1);
    assert_eq!(CecVersion::V1_4.raw(), 0x05);
    assert_eq!(Opcode::ImageViewOn.raw(), 0x04);
}

#[test]
fn unknown_values_arrive_as_data_rather_than_errors() {
    // A device may put anything on the bus; nothing here should panic.
    assert_eq!(Opcode::from_raw(0x7F), Opcode::Other(0x7F));
    assert_eq!(LogicalAddress::from_raw(99), LogicalAddress::Other(99));
    assert_eq!(Opcode::Other(0x7F).raw(), 0x7F);
    assert_eq!(LogLevel::from_raw(1 << 20).raw(), 1 << 20);
}

#[test]
fn enum_display_comes_from_libcec() {
    assert_eq!(LogicalAddress::Tv.to_string(), "TV");
    assert_eq!(LogicalAddress::AudioSystem.to_string(), "Audio");
    assert_eq!(VendorId::PulseEight.to_string(), "Pulse Eight");
    assert_eq!(
        AdapterType::P8External.to_string(),
        "Pulse-Eight USB-CEC Adapter"
    );
    // Unknown values still format, rather than running off the end of a table.
    assert!(!Opcode::Other(0x7F).to_string().is_empty());
}

#[test]
fn commands_round_trip_through_the_c_struct() {
    let command = Command::new(LogicalAddress::Tv, Opcode::SetOsdString)
        .from_initiator(LogicalAddress::PlaybackDevice1)
        .with_parameters(vec![0x00, b'h', b'i'])
        .with_timeout(Duration::from_millis(2500));

    let raw = command.to_raw().expect("a 3-byte payload fits");
    let back = Command::from_raw(&raw);

    assert_eq!(
        back, command,
        "a command should survive the C struct intact"
    );
    assert_eq!(back.initiator, LogicalAddress::PlaybackDevice1);
    assert_eq!(back.destination, LogicalAddress::Tv);
    assert_eq!(back.opcode, Opcode::SetOsdString);
    assert_eq!(back.parameters, vec![0x00, b'h', b'i']);
    assert!(back.opcode_set);
    assert_eq!(back.transmit_timeout, Duration::from_millis(2500));
}

#[test]
fn a_poll_carries_no_opcode() {
    let poll = Command::poll(LogicalAddress::Tv);
    assert!(!poll.opcode_set);
    let raw = poll.to_raw().unwrap();
    assert_eq!(raw.opcode_set, 0);
    assert_eq!(raw.parameters.size, 0);
    // The address names come from libCEC's own tables, casing and all.
    assert_eq!(poll.to_string(), "unknown -> TV POLL");
}

#[test]
fn oversized_parameters_are_refused_not_truncated() {
    let too_long = vec![0u8; libcec::ffi::CEC_MAX_DATA_PACKET_SIZE + 1];
    let command = Command::new(LogicalAddress::Tv, Opcode::VendorCommand).with_parameters(too_long);
    assert_eq!(
        command.to_raw().unwrap_err(),
        Error::ParametersTooLong(libcec::ffi::CEC_MAX_DATA_PACKET_SIZE + 1)
    );

    // Exactly at the limit is fine.
    let full = vec![0u8; libcec::ffi::CEC_MAX_DATA_PACKET_SIZE];
    assert!(Command::new(LogicalAddress::Tv, Opcode::VendorCommand)
        .with_parameters(full)
        .to_raw()
        .is_ok());
}

#[test]
fn a_device_name_too_long_for_the_field_is_rejected() {
    // LIBCEC_OSD_NAME_SIZE is 15, so 14 characters plus a terminator.
    let err = ConnectionBuilder::new("a".repeat(15))
        .open(Some("does-not-exist"), Duration::from_millis(1))
        .unwrap_err();
    assert!(
        matches!(
            err,
            Error::InvalidString {
                field: "device name",
                ..
            }
        ),
        "expected a device-name error, got {err:?}"
    );

    // One character shorter fails later, at the port, not at the name.
    let err = ConnectionBuilder::new("a".repeat(14))
        .open(Some("does-not-exist"), Duration::from_millis(1))
        .unwrap_err();
    assert!(
        !matches!(
            err,
            Error::InvalidString {
                field: "device name",
                ..
            }
        ),
        "14 characters should fit, got {err:?}"
    );
}

#[test]
fn audio_status_splits_into_mute_and_volume() {
    assert_eq!(AudioStatus(0x00).volume(), Some(0));
    assert_eq!(AudioStatus(0x32).volume(), Some(50));
    assert!(!AudioStatus(0x32).is_muted());

    // The top bit is the mute flag, the low seven are the volume.
    let muted = AudioStatus(0x80 | 0x32);
    assert!(muted.is_muted());
    assert_eq!(muted.volume(), Some(50));
    assert_eq!(muted.to_string(), "muted (volume 50)");

    // 0x7F in the volume bits is "unknown", not a volume of 127.
    assert_eq!(AudioStatus::UNKNOWN.volume(), None);
    assert_eq!(AudioStatus::UNKNOWN.to_string(), "unknown");
}

#[test]
fn keypress_tells_a_press_from_a_release() {
    let press = Keypress {
        keycode: UserControlCode::Select,
        duration: Duration::ZERO,
    };
    assert!(press.is_press());
    assert_eq!(press.to_string(), "select pressed");

    let release = Keypress {
        keycode: UserControlCode::Select,
        duration: Duration::from_millis(400),
    };
    assert!(!release.is_press());
}

#[test]
fn physical_addresses_format_the_way_hdmi_writes_them() {
    assert_eq!(libcec::format_physical_address(0x0000), "0.0.0.0");
    assert_eq!(libcec::format_physical_address(0x1000), "1.0.0.0");
    assert_eq!(libcec::format_physical_address(0x1234), "1.2.3.4");
}

#[test]
fn errors_say_which_call_failed() {
    assert_eq!(
        Error::Call("transmit a command").to_string(),
        "libCEC refused to transmit a command"
    );
    assert_eq!(
        Error::Open(Some("COM3".into())).to_string(),
        "could not open the CEC adapter on COM3"
    );
    assert_eq!(
        Error::Open(None).to_string(),
        "could not open a CEC adapter"
    );
}

// ---------------------------------------------------------------------------
// hardware - skipped when no adapter is attached
// ---------------------------------------------------------------------------

/// An adapter can only be open once, and cargo runs the tests in one binary on
/// several threads, so the hardware tests have to take turns.
static ADAPTER: Mutex<()> = Mutex::new(());

/// Skip the body when there is no adapter, so this suite is honest on a build
/// machine and still exercises the real thing on a bench. Yields the adapters
/// and the turn-taking guard, which has to stay alive for the whole test.
macro_rules! require_adapter {
    () => {{
        // A test that panicked while holding the lock poisoned it without
        // leaving anything behind - it dropped its connections on the way out.
        let guard = ADAPTER
            .lock()
            .unwrap_or_else(|poisoned| poisoned.into_inner());
        let adapters = Connection::detect_adapters(true).expect("adapter detection works");
        if adapters.is_empty() {
            eprintln!("no CEC adapter attached - skipping");
            return;
        }
        (adapters, guard)
    }};
}

#[test]
fn detect_adapters_describes_what_it_finds() {
    let (adapters, _guard) = require_adapter!();
    for adapter in &adapters {
        println!("{adapter}");
        assert!(!adapter.port.is_empty(), "an adapter needs a port to open");
        assert_ne!(
            adapter.adapter_type,
            AdapterType::Unknown,
            "adapter type should be identified"
        );
    }
}

#[test]
fn open_query_and_drop() {
    let (adapters, _guard) = require_adapter!();
    let port = adapters[0].port.clone();

    let (handler, events) = channel();
    let cec = ConnectionBuilder::new("RustCEC")
        .device_type(DeviceType::RecordingDevice)
        // Emphatically not the default: opening a connection should not take
        // over the television of whoever is running the tests.
        .activate_source(false)
        .callbacks(handler)
        .open(Some(&port), Duration::from_secs(10))
        .expect("the adapter detected a moment ago should open");

    println!("lib info: {}", cec.lib_info());
    assert!(cec.lib_info().contains("features"));

    let addresses = cec.logical_addresses();
    println!("claimed: {addresses:?}");
    assert!(
        !addresses.addresses.is_empty(),
        "a non-monitoring client should hold an address"
    );

    let config = cec.configuration().expect("configuration is readable");
    assert_eq!(config.device_name, "RustCEC");
    println!(
        "physical address {}",
        libcec::format_physical_address(config.physical_address)
    );

    println!("active source: {}", cec.active_source());
    println!("TV power: {}", cec.power_status(LogicalAddress::Tv));
    println!("TV name: {:?}", cec.device_osd_name(LogicalAddress::Tv));
    println!("active devices: {:?}", cec.active_devices());

    // Opening the connection always logs, so the callback path is proven by the
    // channel having received something at all.
    drop(cec);
    let delivered = events.try_iter().count();
    println!("events delivered: {delivered}");
    assert!(delivered > 0, "expected at least one log message");
}

#[test]
fn an_adapter_in_use_is_skipped_rather_than_fought_over() {
    let (adapters, _guard) = require_adapter!();

    // Hold every adapter this machine has, leaving the open below nothing it can
    // pick. Whichever process opens an adapter first owns the port until it lets
    // go, on Windows and on Linux alike.
    let held: Vec<_> = adapters
        .iter()
        .filter_map(|adapter| {
            ConnectionBuilder::new("RustCEC")
                .activate_source(false)
                .open(Some(&adapter.port), Duration::from_secs(10))
                .ok()
        })
        .collect();
    assert!(!held.is_empty(), "a detected adapter should open");

    // Every adapter is tried before this gives up, and the error names no port
    // because libCEC was the one choosing.
    match ConnectionBuilder::new("RustCEC")
        .activate_source(false)
        .open(None, Duration::from_secs(5))
    {
        Err(Error::Open(None)) => {}
        Err(other) => panic!("unexpected error: {other}"),
        Ok(_) => panic!("an adapter that is in use should not open a second time"),
    }
}

#[test]
fn trait_callbacks_are_called_on_libcecs_thread() {
    let (_adapters, _guard) = require_adapter!();

    #[derive(Default)]
    struct Counter {
        logs: AtomicUsize,
    }

    impl CecCallbacks for Counter {
        fn log_message(&self, _message: &libcec::LogMessage) {
            self.logs.fetch_add(1, Ordering::Relaxed);
        }
    }

    let counter = Arc::new(Counter::default());
    let cec = ConnectionBuilder::new("RustCEC")
        .activate_source(false)
        .callbacks(counter.clone())
        .open_first()
        .expect("open");

    // Dropping joins libCEC's worker thread, so no callback can be in flight
    // after this returns - which is what makes reading the count race-free.
    drop(cec);

    assert!(
        counter.logs.load(Ordering::Relaxed) > 0,
        "libCEC logs while opening, so the trait should have been called"
    );
}
