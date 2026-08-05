# libcec — Rust binding

Rust binding for [libCEC](https://github.com/Pulse-Eight/libcec): control
CEC-capable HDMI hardware (TVs, AV receivers, set-top boxes) over a Pulse-Eight
USB-CEC adapter or a SoC-native CEC backend.

It binds libCEC through its C API (`include/cecc.h`) — the same surface the .NET
and Node.js bindings use — so every language sees one engine and one set of
protocol types.

> **Status: work in progress.** The raw FFI layer (`libcec::ffi`) is complete and
> layout-verified. The safe API on top of it is still being written.

## Installing libCEC

This crate links libCEC; it does not build or vendor it.

| Platform | How |
|---|---|
| Debian / Ubuntu | `apt install libcec8-dev` |
| macOS | `brew install libcec` |
| Windows | the [USB-CEC Adapter SDK](https://github.com/Pulse-Eight/libcec/releases) installer |
| from source | [docs/README.linux.md](../../docs/README.linux.md) |

**The major version has to match.** `libcec_configuration` gains fields on every
major bump, so this crate's `8.x` releases require a libCEC `8.x`. `build.rs`
checks it through `pkg-config` and stops the build on a mismatch rather than
letting a wrong-sized struct reach libCEC.

## Building

On Unix, `pkg-config` finds the library and nothing else is needed:

```sh
cargo build
```

To build against a libCEC that is not installed — this repo's own build tree, or
the Windows SDK — point `LIBCEC_LIB_DIR` at the directory holding `cec.lib` /
`libcec.so`. It overrides `pkg-config` on every platform:

```sh
# a local cmake build
LIBCEC_LIB_DIR=/path/to/libcec/build/src/libcec cargo build
```

```powershell
# Windows, against a repo build (PowerShell)
$env:LIBCEC_LIB_DIR = "C:\dev\libcec\build\Release\x64\lib"
cargo build
```

Windows falls back to the installed SDK (`C:\Program Files\Pulse-Eight\USB-CEC
Adapter`) when `LIBCEC_LIB_DIR` is unset, matching what `binding.gyp` does for
the Node.js addon. `cec.dll` must be on `PATH` at run time.

## Layout

| Path | What |
|---|---|
| `src/ffi.rs` | raw C declarations, mirroring `cecc.h` + `cectypes.h` |
| `src/lib.rs` | crate root and documentation |
| `tests/layout.rs` | asserts every struct size and field offset against the headers |

The FFI is written by hand rather than generated, which is what keeps the crate
**dependency-free**: `cargo build --offline` works, so cmake and the Debian
package can build it with no network and no vendored registry. `tests/layout.rs`
is the safety net — its numbers come from a C compiler reading the real headers,
and it names the field when one drifts.

```sh
cargo test        # needs libCEC installed: the test binary links it
```

## Licence

GPL-2.0-or-later, or a commercial licence from
[Pulse-Eight](http://www.pulse-eight.com/) — the same dual licence as libCEC
itself.
