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

//! Minimal smoke test, mirroring the other language samples: open the first
//! adapter, log traffic, list the devices on the bus, then power the TV on.
//!
//!     cargo run --example simple
//!
//! Ctrl-C to quit.

use std::time::Duration;

use libcec::callbacks::channel;
use libcec::enums::{DeviceType, LogLevel, LogicalAddress};
use libcec::{CecEvent, Connection, ConnectionBuilder};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let adapters = Connection::detect_adapters(false)?;
    if adapters.is_empty() {
        eprintln!("no CEC adapters found");
        std::process::exit(1);
    }
    for adapter in &adapters {
        println!("found {adapter}");
    }

    // Whichever of those opens - open() with a port picks a specific one.
    println!("opening ...");
    let (handler, events) = channel();
    let cec = ConnectionBuilder::new("RustCEC")
        .device_type(DeviceType::RecordingDevice)
        // Leave the television's input where the user left it.
        .activate_source(false)
        .callbacks(handler)
        .open(None, Duration::from_secs(10))?;

    println!("{}", cec.lib_info());

    println!("devices on the bus:");
    for address in cec.active_devices().addresses {
        println!(
            "  {address}: {} [{}]",
            cec.device_osd_name(address),
            cec.power_status(address)
        );
    }

    println!("powering on the TV ...");
    cec.power_on(LogicalAddress::Tv)?;

    // Stream events until Ctrl-C. The receiver ends when the connection is
    // dropped, which for this example means never - so Ctrl-C is the exit.
    println!("watching the bus, Ctrl-C to quit");
    for event in events {
        match event {
            // The debug levels are extremely chatty; Notice and worse only.
            // Compare raw values: cec_log_level is a bitmask whose numeric order
            // is severity, which is not what a derived Ord would give.
            CecEvent::LogMessage(message) if message.level.raw() <= LogLevel::Notice.raw() => {
                println!("[cec] {}", message.message);
            }
            CecEvent::KeyPress(key) => println!("key: {key}"),
            CecEvent::Command(command) => println!("cmd: {command}"),
            CecEvent::SourceActivated { address, activated } => {
                println!(
                    "source {address} {}",
                    if activated {
                        "activated"
                    } else {
                        "deactivated"
                    }
                );
            }
            CecEvent::Alert(alert) => println!("alert: {alert:?}"),
            _ => {}
        }
    }

    Ok(())
}
