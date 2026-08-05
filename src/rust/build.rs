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

//! Locate the libCEC to link against, the same way `src/nodejs/binding.gyp`
//! does: `pkg-config` on Unix, and on Windows the `LIBCEC_LIB_DIR` /
//! `LIBCEC_INCLUDE_DIR` environment variables falling back to the installed
//! Pulse-Eight USB-CEC Adapter SDK. `LIBCEC_LIB_DIR` overrides pkg-config
//! everywhere, which is what a build against an uninstalled repo build needs.
//!
//! It never panics when libCEC cannot be found. `cargo doc` and `cargo check`
//! have no reason to need the library, and the docs CI job builds the reference
//! on a runner that has no libCEC installed; a missing library shows up at link
//! time with a clear linker error instead.

use std::env;
use std::process::Command;

/// The libCEC major version this crate mirrors. `libcec_configuration` gains
/// fields at the end on every major bump (`bAutonomousMode`, `iButtonRepeatDelayMs`
/// and `iDeviceVendorId` arrived in 8.0.0), so linking a different major would
/// hand libCEC a struct of the wrong size - silently, and only some of the time.
const REQUIRED_MAJOR: &str = "8";

fn main() {
    println!("cargo:rerun-if-env-changed=LIBCEC_LIB_DIR");
    println!("cargo:rerun-if-env-changed=LIBCEC_INCLUDE_DIR");
    println!("cargo:rerun-if-env-changed=PKG_CONFIG_PATH");
    println!("cargo:rerun-if-changed=build.rs");

    // An explicit lib dir wins everywhere: it is how a build against this repo's
    // own build tree (or the Windows SDK) points at a libCEC that no pkg-config
    // knows about.
    if let Some(dir) = env::var_os("LIBCEC_LIB_DIR") {
        println!("cargo:rustc-link-search=native={}", dir.to_string_lossy());
        link_default();
        return;
    }

    if cfg!(target_os = "windows") {
        // No pkg-config on Windows. The installer lays cec.lib down in the SDK
        // root, which is also what binding.gyp defaults to.
        println!("cargo:rustc-link-search=native=C:\\Program Files\\Pulse-Eight\\USB-CEC Adapter");
        link_default();
        return;
    }

    if pkg_config().is_none() {
        println!(
            "cargo:warning=pkg-config could not describe libcec; linking -lcec from the \
             default search path. Install libcec8-dev, or set LIBCEC_LIB_DIR to a build tree."
        );
        link_default();
    }
}

fn link_default() {
    println!("cargo:rustc-link-lib=dylib=cec");
}

/// Ask pkg-config for libCEC and turn its answer into cargo directives. Done by
/// hand rather than with the pkg-config crate so this crate keeps zero
/// dependencies - see the note in Cargo.toml.
fn pkg_config() -> Option<()> {
    let pkg_config = env::var("PKG_CONFIG").unwrap_or_else(|_| "pkg-config".to_string());

    // Check the major first: a mismatch is a hard error, because it produces a
    // binary that misbehaves rather than one that fails to link. A pkg-config
    // that cannot answer at all is not an error - see the caller.
    let version = run(&pkg_config, &["--modversion", "libcec"])?;
    let version = version.trim();
    let major = version.split('.').next().unwrap_or("");
    if !major.is_empty() && major != REQUIRED_MAJOR {
        panic!(
            "this crate mirrors libCEC {REQUIRED_MAJOR}.x, but pkg-config reports libcec \
             {version}. The libcec_configuration layout differs between majors, so linking \
             these together would corrupt the configuration passed to libCEC. Install a \
             matching libCEC, or point LIBCEC_LIB_DIR at one."
        );
    }

    let libs = run(&pkg_config, &["--libs", "libcec"])?;
    for flag in libs.split_whitespace() {
        if let Some(dir) = flag.strip_prefix("-L") {
            println!("cargo:rustc-link-search=native={dir}");
        } else if let Some(lib) = flag.strip_prefix("-l") {
            println!("cargo:rustc-link-lib=dylib={lib}");
        }
    }

    // Expose the include dir to dependent build scripts (the `links` key routes
    // DEP_CEC_INCLUDE to them), so a crate that also compiles C against libCEC
    // does not have to rerun pkg-config itself.
    if let Some(cflags) = run(&pkg_config, &["--cflags", "libcec"]) {
        let includes: Vec<&str> = cflags
            .split_whitespace()
            .filter_map(|f| f.strip_prefix("-I"))
            .collect();
        if !includes.is_empty() {
            println!("cargo:include={}", includes.join(";"));
        }
    }

    Some(())
}

fn run(program: &str, args: &[&str]) -> Option<String> {
    let out = Command::new(program).args(args).output().ok()?;
    if !out.status.success() {
        return None;
    }
    String::from_utf8(out.stdout).ok()
}
