# libcec — Rust binding

Rust binding for [libCEC](https://github.com/Pulse-Eight/libcec): control
CEC-capable HDMI hardware (TVs, AV receivers, set-top boxes) over a Pulse-Eight
USB-CEC adapter or a SoC-native CEC backend.

It binds libCEC through its C API (`include/cecc.h`) — the same surface the .NET
and Node.js bindings use — so every language sees one engine and one set of
protocol types.

Full reference: **[pulse-eight.github.io/libcec/rust](https://pulse-eight.github.io/libcec/rust/)**

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

## Using it

```rust
use libcec::{ConnectionBuilder, enums::LogicalAddress};

let cec = ConnectionBuilder::new("RustCEC")
    .hdmi_port(1)
    .activate_source(false)
    .open_first()?;

cec.power_on(LogicalAddress::Tv)?;
println!("TV is {}", cec.power_status(LogicalAddress::Tv));
```

libCEC reports keypresses, messages and alerts from its own worker thread. Take
delivery on a channel:

```rust
use libcec::{callbacks::channel, CecEvent, ConnectionBuilder};

let (handler, events) = channel();
let _cec = ConnectionBuilder::new("RustCEC").callbacks(handler).open_first()?;

for event in events {
    if let CecEvent::KeyPress(key) = event {
        println!("{key}");
    }
}
```

...or implement `CecCallbacks` and be called directly on libCEC's thread, which
is the only way to *answer* the two deciding callbacks (`menu_state_changed` and
`command_handler`). See the `callbacks` module for the trade-off.

Two runnable examples:

```sh
cargo run --example simple                  # open, list the bus, stream events
cargo run --example cec_client -- --help    # an interactive console
```

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

The crate is also a cmake option in the main build: `-DENABLE_RUST_LIB=1` builds
it and installs the sources under `share/cargo/registry`, which is what the
Debian `librust-libcec-dev` package ships.

## Layout

| Path | What |
|---|---|
| `src/lib.rs` | crate root: `Connection`, `ConnectionBuilder`, the owned types |
| `src/ffi.rs` | raw C declarations, mirroring `cecc.h` + `cectypes.h` |
| `src/enums.rs` | **generated** — see below |
| `src/callbacks.rs` | the `CecCallbacks` trait and the channel adapter |
| `tests/layout.rs` | asserts every struct size and field offset against the headers |
| `tests/smoke.rs` | calls the linked library for real |
| `tests/safe_api.rs` | the safe layer; hardware tests skip without an adapter |

The FFI is written by hand rather than generated, which is what keeps the crate
**dependency-free**: `cargo build --offline` works, so cmake and the Debian
package can build it with no network and no vendored registry. `tests/layout.rs`
is the safety net — its numbers come from a C compiler reading the real headers,
and it names the field when one drifts.

`src/enums.rs` *is* generated, by
[`support/generate-rust-enums.py`](../../support/generate-rust-enums.py), because
290-odd protocol constants are not a quantity of magic numbers to transcribe by
hand. Re-run it after adding a value to `cectypes.h` and commit the result; it
runs `rustfmt` itself.

```sh
cargo test        # needs libCEC installed: the test binaries link it
```

The hardware tests skip when no adapter is attached, and none of them powers a
device on or off or activates a source — running the suite cannot take over the
television of whoever runs it.

## Publishing

The crate is `libcec` on crates.io (`libcec-sys` and `cec-rs` there are an
unrelated third-party binding), published from 8.1.6 onwards. `cargo publish` is
a deliberate, manual step: a published version is permanent, and can only be
yanked, never replaced. `support/release.py` checks `Cargo.toml`'s version
against `CMakeLists.txt` but does not publish.

`cargo publish` verifies by building, so it needs a libCEC of the same major on
the box — point `LIBCEC_LIB_DIR` at one when the installed library is older than
the crate.

## Licence

GPL-2.0-or-later, or a commercial licence from
[Pulse-Eight](http://www.pulse-eight.com/) — the same dual licence as libCEC
itself.
