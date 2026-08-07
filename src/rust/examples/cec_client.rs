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

//! An interactive CEC console, in the shape of `cec-client` and the Node
//! binding's `cec-client.js`. This is the manual smoke test: the repo has no
//! automated suite for CEC behaviour, because that needs real hardware on a
//! real HDMI bus.
//!
//!     cargo run --example cec_client -- --help
//!     cargo run --example cec_client -- -p 1
//!
//! Type `h` at the prompt for the command list.

use std::io::{self, BufRead, Write};
use std::sync::mpsc::Receiver;
use std::thread;
use std::time::Duration;

use libcec::callbacks::channel;
use libcec::enums::{DeviceType, DisplayControl, LogicalAddress, Opcode, UserControlCode};
use libcec::{format_physical_address, CecEvent, Command, Connection, ConnectionBuilder, Error};

struct Options {
    port: Option<String>,
    hdmi_port: Option<u8>,
    base_device: Option<LogicalAddress>,
    device_type: DeviceType,
    monitor_only: bool,
    list_devices: bool,
    info: bool,
    log_level: i32,
}

impl Default for Options {
    fn default() -> Self {
        Options {
            port: None,
            hdmi_port: None,
            base_device: None,
            device_type: DeviceType::RecordingDevice,
            monitor_only: false,
            list_devices: false,
            info: false,
            // Error | Warning | Notice - the same default cec-client uses.
            log_level: 7,
        }
    }
}

fn print_help() {
    println!(
        "\
usage: cec_client [options]

options:
  -p, --port <n>       HDMI port the adapter is plugged into (1-based)
  -b, --base <addr>    logical address of the device it is plugged into
  -t, --type <type>    tv | recording | tuner | playback | audio
  -s, --serial <port>  open this port instead of the first adapter found
  -m, --monitor        watch the bus without claiming an address
  -l, --list-devices   list adapters and exit
  -i, --info           print adapter info and exit
  -d, --log-level <n>  log bitmask: 1 error, 2 warning, 4 notice, 8 traffic, 16 debug
  -h, --help           this text

commands (type at the prompt):
  tx <bytes>       transmit raw bytes, e.g. tx 10:04
  on [addr]        power on a device (default: TV)
  standby [addr]   put a device in standby (default: TV)
  as               make this client the active source
  is               make this client inactive
  sp <phys>        set the stream path, e.g. sp 1.0.0.0
  spl <addr>       set the stream path to a logical address
  osd <addr> <msg> put a message on a device's display
  key <addr> <n>   send user control code n, then release
  volup/voldown    volume up/down on the amplifier
  mute             toggle mute on the amplifier
  ver <addr>       the CEC version a device speaks
  ven <addr>       a device's vendor id
  pow <addr>       a device's power status
  name <addr>      a device's OSD name
  lang <addr>      a device's menu language
  poll <addr>      poll an address
  scan             scan the bus and describe every device
  self             the addresses this client holds
  stats            adapter frame counters
  mon <0|1>        turn monitoring mode on or off
  ping             check the adapter is answering
  r                rescan the bus
  h, help          this text
  q, quit          exit"
    );
}

fn parse_address(text: &str) -> Option<LogicalAddress> {
    text.parse::<i32>().ok().map(LogicalAddress::from_raw)
}

/// `1.0.0.0` or `1000` -> 0x1000.
fn parse_physical(text: &str) -> Option<u16> {
    if text.contains('.') {
        let parts: Vec<&str> = text.split('.').collect();
        if parts.len() != 4 {
            return None;
        }
        let mut address = 0u16;
        for part in parts {
            let nibble = u16::from_str_radix(part, 16).ok()?;
            if nibble > 0xF {
                return None;
            }
            address = (address << 4) | nibble;
        }
        Some(address)
    } else {
        u16::from_str_radix(text, 16).ok()
    }
}

/// `10:04` or `10 04` -> [0x10, 0x04].
fn parse_bytes(text: &str) -> Option<Vec<u8>> {
    text.split([':', ' ', ','])
        .filter(|part| !part.is_empty())
        .map(|part| u8::from_str_radix(part, 16).ok())
        .collect()
}

fn parse_args() -> Result<Options, String> {
    let mut options = Options::default();
    let mut args = std::env::args().skip(1);
    while let Some(arg) = args.next() {
        let mut value = || args.next().ok_or(format!("{arg} needs a value"));
        match arg.as_str() {
            "-h" | "--help" => {
                print_help();
                std::process::exit(0);
            }
            "-p" | "--port" => options.hdmi_port = Some(value()?.parse().map_err(|_| "bad port")?),
            "-b" | "--base" => {
                options.base_device = Some(parse_address(&value()?).ok_or("bad address")?)
            }
            "-s" | "--serial" => options.port = Some(value()?),
            "-d" | "--log-level" => {
                options.log_level = value()?.parse().map_err(|_| "bad log level")?
            }
            "-t" | "--type" => {
                options.device_type = match value()?.as_str() {
                    "tv" => DeviceType::Tv,
                    "recording" => DeviceType::RecordingDevice,
                    "tuner" => DeviceType::Tuner,
                    "playback" => DeviceType::PlaybackDevice,
                    "audio" => DeviceType::AudioSystem,
                    other => return Err(format!("unknown device type {other}")),
                }
            }
            "-m" | "--monitor" => options.monitor_only = true,
            "-l" | "--list-devices" => options.list_devices = true,
            "-i" | "--info" => options.info = true,
            other => return Err(format!("unknown option {other}")),
        }
    }
    Ok(options)
}

/// Print events as they arrive, on a thread of our own so the prompt stays
/// responsive. This is the reason to prefer the channel form of the callbacks:
/// printing from libCEC's worker thread would hold up the bus.
fn watch(events: Receiver<CecEvent>, log_level: i32) {
    thread::spawn(move || {
        for event in events {
            match event {
                CecEvent::LogMessage(message) => {
                    if message.level.raw() & log_level != 0 {
                        println!("[{:?}] {}", message.level, message.message);
                    }
                }
                CecEvent::KeyPress(key) => println!("key: {key}"),
                CecEvent::Command(command) => println!("cmd: {command}"),
                CecEvent::Alert(alert) => println!("alert: {alert:?}"),
                CecEvent::SourceActivated { address, activated } => println!(
                    "source {address} {}",
                    if activated {
                        "activated"
                    } else {
                        "deactivated"
                    }
                ),
                CecEvent::ConfigurationChanged(config) => println!(
                    "configuration: physical address {}",
                    format_physical_address(config.physical_address)
                ),
                CecEvent::MenuStateChanged(state) => println!("menu: {state}"),
            }
        }
    });
}

fn scan_bus(cec: &Connection) {
    println!("scanning the bus ...");
    for address in cec.active_devices().addresses {
        println!("device #{}: {address}", address.raw());
        println!(
            "  address:       {}",
            format_physical_address(cec.device_physical_address(address))
        );
        println!("  active source: {}", cec.is_active_source(address));
        println!("  vendor:        {:06x}", cec.device_vendor_id(address));
        println!("  osd string:    {}", cec.device_osd_name(address));
        println!("  CEC version:   {}", cec.device_cec_version(address));
        println!("  power status:  {}", cec.power_status(address));
        if let Some(language) = cec.device_menu_language(address) {
            println!("  language:      {language}");
        }
        println!();
    }
}

fn run_command(cec: &Connection, line: &str) -> Result<bool, Error> {
    let mut parts = line.split_whitespace();
    let Some(command) = parts.next() else {
        return Ok(true);
    };
    let rest: Vec<&str> = parts.collect();
    let address = |index: usize, fallback: LogicalAddress| {
        rest.get(index)
            .and_then(|text| parse_address(text))
            .unwrap_or(fallback)
    };

    match command {
        "q" | "quit" => return Ok(false),
        "h" | "help" => print_help(),

        "tx" => match parse_bytes(&rest.join(" ")) {
            // A raw frame is initiator/destination in the first byte, opcode in
            // the second, parameters after that - the same shape cec-client's
            // tx takes.
            Some(bytes) if bytes.len() >= 2 => {
                let command = Command {
                    initiator: LogicalAddress::from_raw((bytes[0] >> 4) as i32),
                    destination: LogicalAddress::from_raw((bytes[0] & 0x0F) as i32),
                    opcode: Opcode::from_raw(bytes[1] as i32),
                    parameters: bytes[2..].to_vec(),
                    ..Command::new(LogicalAddress::Unknown, Opcode::None)
                };
                cec.transmit(&command)?;
                println!("sent {command}");
            }
            _ => println!("usage: tx <bytes>, e.g. tx 10:04"),
        },

        "on" => cec.power_on(address(0, LogicalAddress::Tv))?,
        "standby" => cec.standby(address(0, LogicalAddress::Tv))?,
        "as" => cec.set_active_source(DeviceType::RecordingDevice)?,
        "is" => cec.set_inactive_view()?,

        "sp" => match rest.first().and_then(|text| parse_physical(text)) {
            Some(physical) => cec.set_stream_path(physical)?,
            None => println!("usage: sp <physical address>, e.g. sp 1.0.0.0"),
        },
        "spl" => cec.set_stream_path_to(address(0, LogicalAddress::Tv))?,

        "osd" => {
            if rest.len() < 2 {
                println!("usage: osd <address> <message>");
            } else {
                cec.set_osd_string(
                    address(0, LogicalAddress::Tv),
                    DisplayControl::DisplayForDefaultTime,
                    &rest[1..].join(" "),
                )?;
            }
        }

        "key" => match rest.get(1).and_then(|text| text.parse::<i32>().ok()) {
            Some(code) => {
                let destination = address(0, LogicalAddress::Tv);
                let key = UserControlCode::from_raw(code);
                cec.send_keypress(destination, key, true)?;
                cec.send_key_release(destination, true)?;
                println!("sent {key}");
            }
            None => println!("usage: key <address> <user control code>"),
        },

        "volup" => println!("audio: {}", cec.volume_up(true)),
        "voldown" => println!("audio: {}", cec.volume_down(true)),
        "mute" => println!("audio: {}", cec.mute_audio(true)),

        "ver" => println!("{}", cec.device_cec_version(address(0, LogicalAddress::Tv))),
        "ven" => println!(
            "{:06x}",
            cec.device_vendor_id(address(0, LogicalAddress::Tv))
        ),
        "pow" => println!("{}", cec.power_status(address(0, LogicalAddress::Tv))),
        "name" => println!("{}", cec.device_osd_name(address(0, LogicalAddress::Tv))),
        "lang" => match cec.device_menu_language(address(0, LogicalAddress::Tv)) {
            Some(language) => println!("{language}"),
            None => println!("unknown"),
        },
        "poll" => println!(
            "{}",
            if cec.poll_device(address(0, LogicalAddress::Tv)) {
                "ok"
            } else {
                "no answer"
            }
        ),

        "scan" => scan_bus(cec),
        "self" => {
            let addresses = cec.logical_addresses();
            println!("primary: {}", addresses.primary);
            for address in addresses.addresses {
                println!("  {address}");
            }
        }
        "stats" => println!("{:?}", cec.stats()?),
        "mon" => cec.switch_monitoring(rest.first() != Some(&"0"))?,
        "ping" => {
            cec.ping_adapter()?;
            println!("ok");
        }
        "r" => {
            cec.rescan_devices();
            println!("rescanned");
        }

        other => println!("unknown command '{other}' - type h for help"),
    }
    Ok(true)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let options = match parse_args() {
        Ok(options) => options,
        Err(message) => {
            eprintln!("error: {message}");
            eprintln!("try --help");
            std::process::exit(2);
        }
    };

    if options.list_devices {
        for adapter in Connection::detect_adapters(false)? {
            println!("{adapter}");
            println!("  path:     {}", adapter.path);
            println!("  vendor:   {:04x}", adapter.vendor_id);
            println!("  product:  {:04x}", adapter.product_id);
            println!("  firmware: {}", adapter.firmware_version);
        }
        return Ok(());
    }

    let (handler, events) = channel();
    let mut builder = ConnectionBuilder::new("RustCEC")
        .device_type(options.device_type)
        .monitor_only(options.monitor_only)
        // Opening a console should not change what the television is showing.
        .activate_source(false)
        .callbacks(handler);
    if let Some(port) = options.hdmi_port {
        builder = builder.hdmi_port(port);
    }
    if let Some(base) = options.base_device {
        builder = builder.base_device(base);
    }

    watch(events, options.log_level);

    let cec = builder.open(options.port.as_deref(), Duration::from_secs(10))?;
    println!("{}", cec.lib_info());

    if options.info {
        println!("vendor:  {:04x}", cec.adapter_vendor_id());
        println!("product: {:04x}", cec.adapter_product_id());
        println!("{:?}", cec.configuration()?);
        return Ok(());
    }

    println!("type h for help, q to quit");
    let stdin = io::stdin();
    loop {
        print!("cec> ");
        io::stdout().flush()?;
        let mut line = String::new();
        if stdin.lock().read_line(&mut line)? == 0 {
            break; // EOF
        }
        // Strip a UTF-8 BOM: piping a script into this on Windows puts one on
        // the first line, and "﻿self" is a baffling way to be told a command
        // does not exist.
        let line = line.trim_start_matches('\u{feff}').trim();
        match run_command(&cec, line) {
            Ok(true) => {}
            Ok(false) => break,
            // A command that libCEC refused is not a reason to quit the console.
            Err(error) => println!("error: {error}"),
        }
    }

    println!("closing ...");
    Ok(())
}
