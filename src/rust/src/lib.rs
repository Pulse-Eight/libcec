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
//! # Layout
//!
//! * [`ffi`] - the raw C declarations, mirroring `cecc.h` and `cectypes.h`.
//!   Complete, `unsafe`, and stable enough to build on directly if the safe
//!   layer does not expose what you need.
//!
//! The safe API is being built on top of this; until it lands, [`ffi`] is the
//! whole of the crate.

#![doc(html_logo_url = "https://pulse-eight.github.io/libcec/assets/pulse-eight-logo.png")]
#![warn(missing_docs)]

pub mod ffi;
