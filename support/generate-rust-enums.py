#!/usr/bin/env python3
'''Generate src/rust/src/enums.rs from include/cectypes.h.

The protocol enums are large - roughly 250 constants across 18 enums - and every
one of them has to carry the exact numeric value libCEC uses. Transcribing that
by hand is how a binding ends up sending CEC_OPCODE_PLAY when it meant
CEC_OPCODE_DECK_CONTROL, so it is generated instead, once, and the output is
checked in: cargo has no Python at build time and crates.io ships sources.

    python support/generate-rust-enums.py

Re-run it after adding a value to cectypes.h, and commit the result.

Two things the C header does that Rust cannot follow directly:

* **Duplicate values.** CEC makes UNREGISTERED and BROADCAST both 15, and the
  audio-status masks overlap. A Rust enum needs distinct discriminants, so the
  first name wins the variant and the rest become associated constants that
  alias it - same value, both names still spelled out.
* **Open sets.** Nothing stops a device putting an opcode on the bus that this
  header has never heard of. Every generated enum therefore carries an
  `Other(i32)` catch-all and a total `from_raw`, so an unknown value is data
  rather than a panic or a Result nobody wants to unwrap.
'''
import re
import shutil
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
HEADER = REPO / 'include' / 'cectypes.h'
OUTPUT = REPO / 'src' / 'rust' / 'src' / 'enums.rs'

# (C enum, Rust name, prefix to strip, one-line doc)
ENUMS = [
    ('cec_abort_reason', 'AbortReason', 'CEC_ABORT_REASON_',
     'Why a device refused a command.'),
    ('cec_adapter_type', 'AdapterType', 'ADAPTERTYPE_',
     'Which backend an adapter is driven by.'),
    ('cec_bus_device_status', 'BusDeviceStatus', 'CEC_DEVICE_STATUS_',
     'What libCEC knows about a device on the bus.'),
    ('cec_deck_control_mode', 'DeckControlMode', 'CEC_DECK_CONTROL_MODE_',
     'Deck transport control.'),
    ('cec_deck_info', 'DeckInfo', 'CEC_DECK_INFO_',
     'The state a deck reports.'),
    ('cec_device_type', 'DeviceType', 'CEC_DEVICE_TYPE_',
     'The device type a client announces on the bus.'),
    ('cec_display_control', 'DisplayControl', 'CEC_DISPLAY_CONTROL_',
     'How long an OSD string stays on screen.'),
    ('cec_log_level', 'LogLevel', 'CEC_LOG_',
     'Severity of a log message from libCEC.'),
    ('cec_logical_address', 'LogicalAddress', 'CECDEVICE_',
     'A CEC logical address - who a message is from or to.'),
    ('cec_menu_state', 'MenuState', 'CEC_MENU_STATE_',
     'Whether the device menu is active.'),
    ('cec_opcode', 'Opcode', 'CEC_OPCODE_',
     'A CEC message opcode.'),
    ('cec_play_mode', 'PlayMode', 'CEC_PLAY_MODE_',
     'How a device should play: direction and speed.'),
    ('cec_power_status', 'PowerStatus', 'CEC_POWER_STATUS_',
     'The power state a device reports.'),
    ('cec_system_audio_status', 'SystemAudioStatus', 'CEC_SYSTEM_AUDIO_STATUS_',
     'Whether system audio mode is engaged.'),
    ('cec_user_control_code', 'UserControlCode', 'CEC_USER_CONTROL_CODE_',
     'A remote control key.'),
    ('cec_vendor_id', 'VendorId', 'CEC_VENDOR_',
     'A CEC vendor id, as reported by a device.'),
    ('cec_version', 'CecVersion', 'CEC_VERSION_',
     'The CEC specification version a device speaks.'),
    ('libcec_alert', 'Alert', 'CEC_ALERT_',
     'An out-of-band notification from libCEC.'),
    ('libcec_parameter_type', 'ParameterType', 'CEC_PARAMETER_TYPE_',
     'The type of the data attached to an [`Alert`].'),
]

# libcec_*_to_string helpers, so Display can defer to libCEC's own strings
# instead of this crate growing a second copy of them that can drift.
DISPLAY_VIA_C = {
    'AdapterType': 'libcec_adapter_type_to_string',
    'CecVersion': 'libcec_cec_version_to_string',
    'DeckControlMode': 'libcec_deck_control_mode_to_string',
    'DeckInfo': 'libcec_deck_status_to_string',
    'LogicalAddress': 'libcec_logical_address_to_string',
    'MenuState': 'libcec_menu_state_to_string',
    'Opcode': 'libcec_opcode_to_string',
    'PowerStatus': 'libcec_power_status_to_string',
    'SystemAudioStatus': 'libcec_system_audio_status_to_string',
    'UserControlCode': 'libcec_user_control_key_to_string',
    'VendorId': 'libcec_vendor_id_to_string',
}

# cec_logical_address runs its words together (CECDEVICE_RECORDINGDEVICE1), so
# splitting on underscores alone yields Recordingdevice1. Split these first, and
# match the names the .NET binding already uses for the same values.
COMPOUNDS = {
    'RECORDINGDEVICE': 'RECORDING_DEVICE',
    'PLAYBACKDEVICE': 'PLAYBACK_DEVICE',
    'AUDIOSYSTEM': 'AUDIO_SYSTEM',
    'FREEUSE': 'FREE_USE',
}

# Enums with a sensible Default, by C name of the variant. Everything else is
# left without one: there is no defensible default opcode.
DEFAULT_VARIANTS = {
    'LogicalAddress': 'CECDEVICE_UNKNOWN',
}

# Names Pascal-casing alone gets wrong.
SPECIAL_VARIANTS = {
    ('CecVersion', 'CEC_VERSION_1_2'):  'V1_2',
    ('CecVersion', 'CEC_VERSION_1_2A'): 'V1_2a',
    ('CecVersion', 'CEC_VERSION_1_3'):  'V1_3',
    ('CecVersion', 'CEC_VERSION_1_3A'): 'V1_3a',
    ('CecVersion', 'CEC_VERSION_1_4'):  'V1_4',
    ('CecVersion', 'CEC_VERSION_2_0'):  'V2_0',
}


def read_enum(text: str, name: str) -> list[tuple[str, int]]:
    '''Every (name, value) in a C enum, with implicit values filled in.'''
    m = re.search(r'typedef\s+enum\s+' + name + r'\s*\{(.*?)\}\s*' + name + r'\s*;',
                  text, re.S)
    if not m:
        raise SystemExit(f'could not find enum {name} in {HEADER}')
    body = re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S)
    body = re.sub(r'//.*', '', body)

    entries, nxt = [], 0
    for item in body.split(','):
        item = item.strip()
        if not item:
            continue
        if '=' in item:
            key, raw = (p.strip() for p in item.split('=', 1))
            value = int(raw, 0)
        else:
            key, value = item, nxt
        entries.append((key, value))
        nxt = value + 1
    return entries


def variant_name(rust_enum: str, c_name: str, prefix: str) -> str:
    if (rust_enum, c_name) in SPECIAL_VARIANTS:
        return SPECIAL_VARIANTS[(rust_enum, c_name)]
    stem = c_name[len(prefix):] if c_name.startswith(prefix) else c_name
    for run_together, split in COMPOUNDS.items():
        stem = stem.replace(run_together, split)
    parts = [p for p in stem.split('_') if p]
    out = ''.join(p[:1].upper() + p[1:].lower() for p in parts)
    if not out:
        out = 'Value'
    if out[0].isdigit():
        out = 'V' + out
    return out


def render(rust_enum: str, doc: str, entries: list[tuple[str, int]],
           prefix: str) -> str:
    seen: dict[int, str] = {}
    variants, aliases = [], []
    for c_name, value in entries:
        name = variant_name(rust_enum, c_name, prefix)
        if value in seen:
            aliases.append((name, seen[value], c_name, value))
        else:
            seen[value] = name
            variants.append((name, value, c_name))

    # Deliberately no PartialOrd/Ord: a derived ordering follows declaration
    # order, not the numeric values, so `level <= Notice` would quietly compare
    # the wrong thing and an ordering over opcodes would mean nothing at all.
    # Compare raw() when an order is actually wanted.
    derives = 'Copy, Clone, PartialEq, Eq, Hash, Debug'
    default_of = DEFAULT_VARIANTS.get(rust_enum)
    if default_of:
        derives += ', Default'

    out = [f'/// {doc}',
           '///',
           f'/// Mirrors the C `{[e for e in ENUMS if e[1] == rust_enum][0][0]}`.',
           f'#[derive({derives})]',
           f'pub enum {rust_enum} {{']
    for name, value, c_name in variants:
        out.append(f'    /// `{c_name}`')
        if c_name == default_of:
            out.append('    #[default]')
        out.append(f'    {name},')
    out.append('    /// A value libCEC reported that this crate has no name for.')
    out.append('    ///')
    out.append('    /// The CEC bus carries whatever devices put on it, so this is')
    out.append('    /// data, not an error.')
    out.append('    Other(i32),')
    out.append('}')
    out.append('')

    out.append(f'impl {rust_enum} {{')
    for name, alias_of, c_name, value in aliases:
        out.append(f'    /// `{c_name}` - the same value as'
                   f' [`{rust_enum}::{alias_of}`] ({value}).')
        out.append(f'    pub const {to_const(name)}: {rust_enum} ='
                   f' {rust_enum}::{alias_of};')
    out.append('    /// The value libCEC uses for this variant.')
    out.append('    pub fn raw(self) -> i32 {')
    out.append('        match self {')
    for name, value, _ in variants:
        out.append(f'            {rust_enum}::{name} => {value},')
    out.append(f'            {rust_enum}::Other(value) => value,')
    out.append('        }')
    out.append('    }')
    out.append('')
    out.append('    /// Read a value libCEC produced. Total: anything unrecognised')
    out.append(f'    /// becomes [`{rust_enum}::Other`].')
    out.append('    pub fn from_raw(value: i32) -> Self {')
    out.append('        match value {')
    for name, value, _ in variants:
        out.append(f'            {value} => {rust_enum}::{name},')
    out.append(f'            other => {rust_enum}::Other(other),')
    out.append('        }')
    out.append('    }')
    out.append('}')
    out.append('')
    out.append(f'impl From<i32> for {rust_enum} {{')
    out.append('    fn from(value: i32) -> Self {')
    out.append('        Self::from_raw(value)')
    out.append('    }')
    out.append('}')
    out.append('')
    out.append(f'impl From<{rust_enum}> for i32 {{')
    out.append(f'    fn from(value: {rust_enum}) -> Self {{')
    out.append('        value.raw()')
    out.append('    }')
    out.append('}')
    out.append('')

    if rust_enum in DISPLAY_VIA_C:
        helper = DISPLAY_VIA_C[rust_enum]
        out.append(f'impl fmt::Display for {rust_enum} {{')
        out.append('    fn fmt(&self, f: &mut fmt::Formatter<\'_>) -> fmt::Result {')
        out.append(f'        f.write_str(&describe(ffi::{helper}, self.raw()))')
        out.append('    }')
        out.append('}')
        out.append('')

    return '\n'.join(out)


def to_const(name: str) -> str:
    '''Pascal variant name -> SCREAMING_SNAKE associated constant.'''
    return re.sub(r'(?<!^)(?=[A-Z])', '_', name).upper()


HEADER_TEXT = '''// This file is part of the libCEC(R) library.
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

// GENERATED FILE - do not edit by hand.
//
// Regenerate with:
//
//     python support/generate-rust-enums.py
//
// The source of truth is include/cectypes.h. The generator is what keeps ~250
// protocol constants honest; see its docstring for why the enums look the way
// they do (Other(i32) catch-alls, aliased duplicate values).

//! The protocol enums: opcodes, logical addresses, key codes, vendor ids and the
//! rest of `cectypes.h`.
//!
//! Every enum here is **total**: [`from_raw`](Opcode::from_raw) never fails, and
//! a value this crate has no name for arrives as `Other(i32)` rather than an
//! error. The CEC bus carries whatever the devices on it decide to send, and a
//! binding that panicked on an unrecognised opcode would be a liability.
//!
//! Where libCEC has a `*_to_string` helper, [`Display`](std::fmt::Display) calls
//! it. The strings therefore come from libCEC itself and cannot drift from the
//! ones `cec-client` prints.
//!
//! ```no_run
//! use libcec::enums::{LogicalAddress, Opcode};
//!
//! assert_eq!(LogicalAddress::Tv.raw(), 0);
//! assert_eq!(LogicalAddress::from_raw(99), LogicalAddress::Other(99));
//! println!("{}", Opcode::ImageViewOn); // asks libCEC for the name
//! ```

use std::fmt;

use crate::ffi;
use crate::util::describe;

'''


def main() -> int:
    text = HEADER.read_text(encoding='utf-8')
    chunks = [HEADER_TEXT]
    total = 0
    for c_name, rust_name, prefix, doc in ENUMS:
        entries = read_enum(text, c_name)
        total += len(entries)
        chunks.append(render(rust_name, doc, entries, prefix))
        print(f'{rust_name:<20} {len(entries):>3} values from {c_name}')
    OUTPUT.write_text('\n'.join(chunks), encoding='utf-8')
    print(f'\nwrote {OUTPUT.relative_to(REPO)} ({total} values, {len(ENUMS)} enums)')

    # Hand the result to rustfmt rather than trying to emit canonical formatting
    # from here: `cargo fmt --check` is part of the build, and a generated file
    # that fails it would be a trap for whoever regenerates next.
    rustfmt = shutil.which('rustfmt')
    if rustfmt:
        subprocess.run([rustfmt, '--edition', '2021', str(OUTPUT)], check=True)
        print('formatted with rustfmt')
    else:
        print('WARNING: rustfmt not found; run `cargo fmt` before committing',
              file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
