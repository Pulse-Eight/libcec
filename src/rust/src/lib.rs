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

//! Rust binding for [libCEC](https://github.com/Pulse-Eight/libcec) - control
//! CEC-capable HDMI hardware (TVs, AV receivers, set-top boxes) from Rust.
//!
//! libCEC talks to the HDMI CEC bus through a Pulse-Eight USB-CEC adapter or a
//! SoC-native backend (Raspberry Pi, Exynos, the Linux CEC framework, and
//! others). This crate binds it over its C API, the same surface the .NET and
//! Node.js bindings use, so every language sees the same engine and the same
//! protocol types.
//!
//! # Adding the crate
//!
//! ```text
//! cargo add libcec
//! ```
//!
//! It has no dependencies of its own. libCEC itself is a separate install.
//!
//! # Installing libCEC
//!
//! This crate links libCEC; it does not build or vendor it.
//!
//! | Platform | How |
//! |---|---|
//! | Debian / Ubuntu | `apt install libcec8-dev` |
//! | macOS | `brew install libcec` |
//! | Windows | the [Pulse-Eight USB-CEC Adapter SDK](https://github.com/Pulse-Eight/libcec/releases) installer |
//! | from source | see the [build instructions](https://github.com/Pulse-Eight/libcec/blob/master/docs/README.linux.md) |
//!
//! On Unix the library is found with `pkg-config`. On Windows, and for a build
//! against an uninstalled libCEC anywhere, set `LIBCEC_LIB_DIR` to the directory
//! holding `cec.lib` / `libcec.so`.
//!
//! **The major version has to match.** `libcec_configuration` grows fields on
//! every major bump, so this crate's `8.x` releases require a libCEC `8.x`;
//! `build.rs` stops the build rather than let a mismatch through.
//!
//! # Getting started
//!
//! Find an adapter, open it, and turn the television on:
//!
//! ```no_run
//! use libcec::{Connection, ConnectionBuilder, enums::LogicalAddress};
//!
//! # fn main() -> Result<(), libcec::Error> {
//! for adapter in Connection::detect_adapters(true)? {
//!     println!("{adapter}");
//! }
//!
//! let cec = ConnectionBuilder::new("RustCEC")
//!     .hdmi_port(1)
//!     .open_first()?;
//!
//! cec.power_on(LogicalAddress::Tv)?;
//! println!("TV is {}", cec.power_status(LogicalAddress::Tv));
//! # Ok(())
//! # }
//! ```
//!
//! # Listening to the bus
//!
//! libCEC reports keypresses, messages and alerts from its own worker thread.
//! Take delivery on a channel:
//!
//! ```no_run
//! use libcec::{callbacks::channel, CecEvent, ConnectionBuilder};
//!
//! # fn main() -> Result<(), libcec::Error> {
//! let (handler, events) = channel();
//! let _cec = ConnectionBuilder::new("RustCEC")
//!     .callbacks(handler)
//!     .open_first()?;
//!
//! for event in events {
//!     match event {
//!         CecEvent::KeyPress(key) if key.is_press() => println!("{}", key.keycode),
//!         CecEvent::Command(command) => println!("{command}"),
//!         _ => {}
//!     }
//! }
//! # Ok(())
//! # }
//! ```
//!
//! ...or implement [`CecCallbacks`] and be called directly on libCEC's thread,
//! which is the only way to *answer* the two deciding callbacks. See the
//! [`callbacks`] module for the trade-off.
//!
//! # Layout
//!
//! | Module | What |
//! |---|---|
//! | (root) | [`Connection`], [`ConnectionBuilder`], and the owned protocol types |
//! | [`enums`] | opcodes, logical addresses, key codes, vendor ids |
//! | [`callbacks`] | [`CecCallbacks`] and the [`channel`](callbacks::channel) adapter |
//! | [`ffi`] | the raw C declarations, for anything the safe layer does not cover |
//!
//! # Thread safety
//!
//! [`Connection`] is [`Send`] and [`Sync`]. libCEC serialises access to a
//! connection internally, which is what lets its C++ API be driven from several
//! threads at once and what its own callback thread relies on.
//!
//! Calling back *into* a connection from inside a callback works, but remember
//! that you are on libCEC's worker thread: a blocking call there stalls the CEC
//! bus, and the two callbacks that return a decision are abandoned after a
//! second. The channel form of the callbacks exists to keep that work on a
//! thread of your own.

#![doc(html_logo_url = "https://pulse-eight.github.io/libcec/assets/pulse-eight-logo.png")]
#![warn(missing_docs)]
#![warn(clippy::undocumented_unsafe_blocks)]

pub mod callbacks;
pub mod enums;
pub mod ffi;

mod connection;
mod error;
mod types;
mod util;

pub use callbacks::{CecCallbacks, CecEvent};
pub use connection::{Connection, ConnectionBuilder, DEFAULT_OPEN_TIMEOUT};
pub use error::{Error, Result};
pub use types::{
    format_physical_address, AdapterDescriptor, AdapterStats, AudioStatus, Command, Configuration,
    Keypress, LogMessage, LogicalAddresses,
};
