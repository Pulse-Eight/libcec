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

//! The safety net under the hand-written FFI in `src/ffi.rs`.
//!
//! Every number below came from a C compiler reading `include/cectypes.h`
//! directly - `sizeof`, `_Alignof` and `offsetof` for each struct the binding
//! mirrors. If a field is added, reordered or given the wrong width on the Rust
//! side, one of these fails and names the field.
//!
//! To regenerate the expectations after an intentional header change, compile a
//! probe against the real headers and read its output:
//!
//! ```sh
//! cat > probe.c <<'EOF'
//! #include <stdio.h>
//! #include <stddef.h>
//! #include "cectypes.h"
//! int main(void) {
//!   printf("%zu %zu\n", sizeof(cec_command), offsetof(cec_command, parameters));
//!   return 0;
//! }
//! EOF
//! cc -I../../include probe.c -o probe && ./probe
//! ```
//!
//! Structs holding a pointer are checked on 64-bit targets only: their layout is
//! genuinely different on 32-bit, and asserting one set of numbers for both would
//! just be wrong. The pointer-free structs - which is most of the protocol
//! surface - are checked everywhere.

use std::mem::{align_of, size_of, MaybeUninit};

use libcec::ffi::*;

/// Byte offset of a field, without ever creating a reference to uninitialised
/// memory. `core::mem::offset_of!` would do this, but it landed in Rust 1.77 and
/// this crate builds on 1.63.
macro_rules! offset_of {
    ($t:ty, $f:ident) => {{
        let uninit = MaybeUninit::<$t>::uninit();
        let base = uninit.as_ptr();
        // SAFETY: addr_of! computes an address without reading the place, and
        // both pointers are derived from the same allocation.
        unsafe { (std::ptr::addr_of!((*base).$f) as usize) - (base as usize) }
    }};
}

/// `check!(Type, size, align, field => offset, ...)`
macro_rules! check {
    ($t:ty, $size:expr, $align:expr $(, $f:ident => $off:expr)* $(,)?) => {
        assert_eq!(size_of::<$t>(), $size, "size of {}", stringify!($t));
        assert_eq!(align_of::<$t>(), $align, "align of {}", stringify!($t));
        $(
            assert_eq!(offset_of!($t, $f), $off,
                       "offset of {}.{}", stringify!($t), stringify!($f));
        )*
    };
}

#[test]
fn sizes_match_the_headers() {
    assert_eq!(CEC_MAX_DATA_PACKET_SIZE, 64);
    assert_eq!(LIBCEC_OSD_NAME_SIZE, 15);
    assert_eq!(CEC_OSD_NAME_SIZE, 14);
    assert_eq!(CEC_MENU_LANGUAGE_SIZE, 4);
    assert_eq!(CEC_DEVICE_TYPE_LIST_SIZE, 5);
    assert_eq!(CEC_LOGICAL_ADDRESS_COUNT, 16);

    // C gives every enum in cectypes.h `int` as its underlying type, because the
    // largest value in any of them (CEC_VENDOR_HARMAN_KARDON, 0x9C645E) fits.
    assert_eq!(size_of::<cec_logical_address>(), 4);
    assert_eq!(size_of::<cec_opcode>(), 4);
    assert_eq!(size_of::<cec_vendor_id>(), 4);
}

#[test]
fn pointer_free_structs_match_the_headers() {
    check!(cec_datapacket, 65, 1, data => 0, size => 64);

    check!(cec_command, 88, 4,
        initiator        => 0,
        destination      => 4,
        ack              => 8,
        eom              => 9,
        opcode           => 12,
        parameters       => 16,
        opcode_set       => 81,
        transmit_timeout => 84,
    );

    check!(cec_keypress, 8, 4, keycode => 0, duration => 4);

    check!(cec_adapter, 2048, 1, path => 0, comm => 1024);

    check!(cec_adapter_descriptor, 2128, 4,
        strComPath         => 0,
        strComName         => 1024,
        iVendorId          => 2048,
        iProductId         => 2050,
        iFirmwareVersion   => 2052,
        iPhysicalAddress   => 2054,
        iFirmwareBuildDate => 2056,
        adapterType        => 2060,
        strDeviceName      => 2064,
    );

    check!(cec_device_type_list, 20, 4, types => 0);

    check!(cec_logical_addresses, 68, 4, primary => 0, addresses => 4);

    check!(cec_adapter_stats, 20, 4);
}

#[cfg(target_pointer_width = "64")]
#[test]
fn pointer_bearing_structs_match_the_headers() {
    check!(cec_log_message, 24, 8, message => 0, level => 8, time => 16);

    check!(libcec_parameter, 16, 8, paramType => 0, paramData => 8);

    // Eight function pointers, in the order libCEC calls them by name. Getting
    // this order wrong would route log messages into the keypress handler.
    check!(ICECCallbacks, 64, 8,
        logMessage           => 0,
        keyPress             => 8,
        commandReceived      => 16,
        configurationChanged => 24,
        alert                => 32,
        menuStateChanged     => 40,
        sourceActivated      => 48,
        commandHandler       => 56,
    );

    check!(libcec_configuration, 344, 8,
        clientVersion         => 0,
        strDeviceName         => 4,
        deviceTypes           => 20,
        bAutodetectAddress    => 40,
        iPhysicalAddress      => 42,
        baseDevice            => 44,
        iHDMIPort             => 48,
        tvVendor              => 52,
        wakeDevices           => 56,
        powerOffDevices       => 124,
        serverVersion         => 192,
        bGetSettingsFromROM   => 196,
        bActivateSource       => 197,
        bPowerOffOnStandby    => 198,
        callbackParam         => 200,
        callbacks             => 208,
        logicalAddresses      => 216,
        iFirmwareVersion      => 284,
        strDeviceLanguage     => 286,
        iFirmwareBuildDate    => 292,
        bMonitorOnly          => 296,
        cecVersion            => 300,
        adapterType           => 304,
        comboKey              => 308,
        iComboKeyTimeoutMs    => 312,
        iButtonRepeatRateMs   => 316,
        iButtonReleaseDelayMs => 320,
        iDoubleTapTimeoutMs   => 324,
        bAutoWakeAVR          => 328,
        bAutoPowerOn          => 329,
        bAutonomousMode       => 330,
        iButtonRepeatDelayMs  => 332,
        iDeviceVendorId       => 336,
    );
}

/// A nullable C function pointer has to stay pointer-sized through `Option`, or
/// the callback table silently grows and every field after it lands in the wrong
/// place. Rust guarantees this niche, but the guarantee is load-bearing here.
#[test]
fn optional_callbacks_stay_pointer_sized() {
    assert_eq!(size_of::<Option<cec_log_message_cb>>(), size_of::<usize>());
    assert_eq!(
        size_of::<Option<cec_command_handler_cb>>(),
        size_of::<usize>()
    );
}

/// `Default` is `mem::zeroed()`, which is only sound if all-zero is a valid
/// value. Check the parts of that promise that are observable: a zeroed
/// callback table is all-`None`, and a zeroed configuration has null pointers.
#[test]
fn zeroed_defaults_are_valid() {
    let callbacks = ICECCallbacks::default();
    assert!(callbacks.logMessage.is_none());
    assert!(callbacks.commandHandler.is_none());

    let config = libcec_configuration::default();
    assert!(config.callbacks.is_null());
    assert!(config.callbackParam.is_null());
    assert_eq!(config.clientVersion, 0);

    let command = cec_command::default();
    assert_eq!(command.parameters.size, 0);
    assert_eq!(command.opcode_set, 0);
}
