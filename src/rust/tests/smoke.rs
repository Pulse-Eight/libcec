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

//! Calls libCEC across the FFI boundary for real.
//!
//! `layout.rs` proves the structs are the right shape; this proves the shape is
//! the one the *linked* library actually uses. It needs libCEC present at run
//! time (on Windows that means `cec.dll` on `PATH`), but no CEC hardware: every
//! call here is answered without an adapter attached.

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::ptr;

use libcec::ffi::*;

/// Build the configuration libCEC expects: zeroed, then handed to
/// `libcec_clear_configuration`, which fills in the defaults and the version.
fn default_config() -> libcec_configuration {
    let mut config = libcec_configuration::default();
    // SAFETY: Clear() only writes, so a zeroed struct is a valid target.
    unsafe { libcec_clear_configuration(&mut config) };
    config
}

fn copy_into(dst: &mut [c_char], text: &str) {
    let bytes = CString::new(text).unwrap();
    let src = bytes.as_bytes_with_nul();
    assert!(src.len() <= dst.len(), "{text} does not fit");
    for (d, s) in dst.iter_mut().zip(src) {
        *d = *s as c_char;
    }
}

/// The defaults libCEC writes are the ones `cectypes.h` documents - and, more to
/// the point, `clientVersion` tells us which major we are actually linked
/// against. `build.rs` can only check that through pkg-config, which does not
/// exist on Windows, so this is the check that covers every platform.
#[test]
fn clear_configuration_fills_in_the_documented_defaults() {
    let config = default_config();

    let major = (config.clientVersion >> 16) & 0xFF;
    assert_eq!(
        major, 8,
        "linked against libCEC {major}.x, but src/ffi.rs mirrors 8.x - the \
         libcec_configuration layout differs between majors"
    );
    assert_eq!(config.serverVersion, config.clientVersion);

    // Clear() sets these explicitly; see libcec_configuration::Clear().
    assert_eq!(config.iPhysicalAddress, 0); // CEC_PHYSICAL_ADDRESS_TV
    assert_eq!(config.baseDevice, 0); // CEC_DEFAULT_BASE_DEVICE
    assert_eq!(config.bAutodetectAddress, 1);
    assert_eq!(config.bMonitorOnly, 0);
    assert_eq!(config.tvVendor, 0); // CEC_VENDOR_UNKNOWN
    assert!(config.callbacks.is_null());

    // 3 characters, not NUL-terminated - the one string field that is not.
    let language: Vec<u8> = config.strDeviceLanguage.iter().map(|c| *c as u8).collect();
    assert_eq!(language.len(), 3);
    assert!(
        language.iter().all(|c| c.is_ascii_lowercase()),
        "expected an ISO 639-2 code, got {language:?}"
    );
}

/// The full open-and-shut cycle over the raw API: initialise, ask libCEC about
/// itself, enumerate adapters, destroy. If any struct in `ffi.rs` were the wrong
/// size, this is where it would corrupt the stack rather than merely disagree.
#[test]
fn initialise_query_and_destroy() {
    let mut config = default_config();
    copy_into(&mut config.strDeviceName, "RustCEC");
    config.deviceTypes.types[0] = 1; // CEC_DEVICE_TYPE_RECORDING_DEVICE

    // SAFETY: config outlives the connection, and carries no callbacks.
    let connection = unsafe { libcec_initialise(&mut config) };
    assert!(!connection.is_null(), "libcec_initialise returned null");

    // SAFETY: connection is non-null and live until libcec_destroy below.
    unsafe {
        let info = libcec_get_lib_info(connection);
        assert!(!info.is_null());
        let info = CStr::from_ptr(info).to_string_lossy().into_owned();
        // The text is whatever cmake baked into LIB_INFO (compiler, host and
        // compiled-in backends), so only the feature list is worth asserting on
        // - it is the one part both LIB_INFO and the fallback string share.
        assert!(info.contains("features"), "unexpected lib info: {info}");
        println!("lib info: {info}");

        // No adapter needed: with none attached this reports 0, not an error.
        let mut adapters = [cec_adapter_descriptor::default(); 4];
        let found = libcec_detect_adapters(
            connection,
            adapters.as_mut_ptr(),
            adapters.len() as u8,
            ptr::null(),
            1, // quick scan: skip probing each port for firmware details
        );
        assert!(found >= 0, "libcec_detect_adapters failed: {found}");
        println!("adapters found: {found}");

        for adapter in adapters.iter().take(found as usize) {
            let path = CStr::from_ptr(adapter.strComPath.as_ptr()).to_string_lossy();
            let port = CStr::from_ptr(adapter.strComName.as_ptr()).to_string_lossy();
            let name = CStr::from_ptr(adapter.strDeviceName.as_ptr()).to_string_lossy();
            println!(
                "  {port} ({path}) {name} vendor={:04x} product={:04x} type={}",
                adapter.iVendorId, adapter.iProductId, adapter.adapterType
            );
        }

        libcec_close(connection);
        libcec_destroy(connection);
    }
}

/// The `*_to_string` helpers write into a caller-supplied buffer. Reading one
/// back proves the `usize` bufsize argument lines up with the C `size_t` - a
/// mismatch there would truncate or overrun.
#[test]
fn enum_formatting_round_trips_through_c() {
    fn to_string(f: unsafe extern "C" fn(i32, *mut c_char, usize), value: i32) -> String {
        let mut buf = [0 as c_char; 64];
        // SAFETY: buf is 64 bytes and we say so.
        unsafe {
            f(value, buf.as_mut_ptr(), buf.len());
            CStr::from_ptr(buf.as_ptr()).to_string_lossy().into_owned()
        }
    }

    // CECDEVICE_TV = 0, CEC_POWER_STATUS_ON = 0x00, CEC_VENDOR_PULSE_EIGHT.
    assert_eq!(to_string(libcec_logical_address_to_string, 0), "TV");
    assert_eq!(to_string(libcec_power_status_to_string, 0x00), "on");
    assert_eq!(
        to_string(libcec_vendor_id_to_string, 0x001582),
        "Pulse Eight"
    );

    // An unknown value must still produce something, not run off the end.
    let unknown = to_string(libcec_opcode_to_string, 0x7F7F);
    assert!(!unknown.is_empty());
}
