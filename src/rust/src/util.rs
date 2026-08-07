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

//! Crossing the C string boundary, in the three shapes libCEC needs it.

use std::os::raw::{c_char, c_int};

use crate::error::Error;

/// Read a fixed-width C buffer that may or may not be NUL-terminated.
///
/// libCEC fills these to capacity and only sometimes leaves room for the
/// terminator - `strDeviceLanguage` is 3 bytes with no terminator at all - so
/// stop at the first NUL *or* the end of the buffer, whichever comes first.
pub(crate) fn read_fixed(buf: &[c_char]) -> String {
    let bytes: Vec<u8> = buf
        .iter()
        .take_while(|c| **c != 0)
        .map(|c| *c as u8)
        .collect();
    String::from_utf8_lossy(&bytes).into_owned()
}

/// Read a NUL-terminated C string from a pointer libCEC owns.
///
/// # Safety
///
/// `ptr` must be null or point at a NUL-terminated string that stays valid for
/// the duration of the call.
pub(crate) unsafe fn read_ptr(ptr: *const c_char) -> String {
    if ptr.is_null() {
        return String::new();
    }
    std::ffi::CStr::from_ptr(ptr).to_string_lossy().into_owned()
}

/// Copy a string into a fixed-width C buffer, always leaving a NUL terminator.
///
/// Fails rather than truncates: a device name silently cut to
/// `LIBCEC_OSD_NAME_SIZE` would show up on the television, and finding out then
/// is worse than finding out here.
pub(crate) fn write_fixed(dst: &mut [c_char], src: &str, field: &'static str) -> Result<(), Error> {
    if src.as_bytes().contains(&0) {
        return Err(Error::InvalidString {
            field,
            reason: "contains an interior NUL byte",
        });
    }
    if src.len() >= dst.len() {
        return Err(Error::InvalidString {
            field,
            reason: "too long for the field libCEC provides",
        });
    }
    for slot in dst.iter_mut() {
        *slot = 0;
    }
    for (slot, byte) in dst.iter_mut().zip(src.as_bytes()) {
        *slot = *byte as c_char;
    }
    Ok(())
}

/// Ask libCEC to name one of its own enum values.
///
/// Every `libcec_*_to_string` helper has this shape. Going through them keeps
/// the crate from carrying a second copy of the name tables, which is a copy
/// that could disagree with what `cec-client` prints for the same value.
pub(crate) fn describe(f: unsafe extern "C" fn(c_int, *mut c_char, usize), value: i32) -> String {
    // 64 bytes is what the Node addon uses; the longest name libCEC produces is
    // well short of it, and the helpers all take the cap as an argument anyway.
    let mut buf = [0 as c_char; 64];
    // SAFETY: buf is 64 c_chars and that is the size we pass.
    unsafe {
        f(value, buf.as_mut_ptr(), buf.len());
        read_ptr(buf.as_ptr())
    }
}

/// C's `int`-as-boolean, on the way out.
pub(crate) fn as_c_bool(value: bool) -> c_int {
    if value {
        1
    } else {
        0
    }
}

/// C's `int`-as-boolean, on the way in.
pub(crate) fn from_c_bool(value: c_int) -> bool {
    value != 0
}
