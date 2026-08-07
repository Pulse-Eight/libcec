# API documentation

Reference documentation for libCEC's public APIs, one best-of-breed generator
per binding, assembled into a single site and published to GitHub Pages by
[`.github/workflows/docs.yml`](../../.github/workflows/docs.yml) on every push to
`master` that touches an API surface.

| Binding | Tool | Source | Config |
|---------|------|--------|--------|
| C / C++ | [Doxygen](https://www.doxygen.nl/) | `include/*.h` | [`doxygen/Doxyfile`](doxygen/Doxyfile) |
| .NET | [DocFX](https://dotnet.github.io/docfx/) | `src/dotnetlib/cs/**` | [`dotnet/docfx.json`](dotnet/docfx.json) |
| Node.js | [TypeDoc](https://typedoc.org/) | `src/nodejs/index.d.ts` | [`nodejs/typedoc.json`](nodejs/typedoc.json) |
| Python | [Sphinx](https://www.sphinx-doc.org/) | SWIG `cec` module from `src/libcec/libcec.i` | [`python/conf.py`](python/conf.py) |
| Rust | [rustdoc](https://doc.rust-lang.org/rustdoc/) | `src/rust/src/**` | the crate's own doc comments |

The [landing page](landing/index.html) links the five together. All bindings
wrap the same core engine (`ICECAdapter` + the protocol types in
`include/cectypes.h`), so the C/C++ reference documents the concepts in the most
depth and the others mirror them.

## How each is wired

- **Doxygen** reads the public headers. `cec.h` is already richly documented;
  `EXTRACT_ALL` also surfaces the (comment-free) C declarations in `cecc.h`. The
  CI job injects `PROJECT_NUMBER`/`OUTPUT_DIRECTORY` on stdin and drops the
  [doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css) theme next
  to the `Doxyfile`.
- **DocFX** compiles a docs-only [`dotnet/docs.csproj`](dotnet/docs.csproj) that
  globs the same `cs/` sources as the real binding (which is generated from a
  `.csproj.in` by cmake) and extracts their XML doc comments. This keeps the
  docs build independent of the cmake configure step.
- **TypeDoc** renders [`src/nodejs/index.d.ts`](../../src/nodejs/index.d.ts) —
  hand-authored TypeScript typings for the addon that are *also* shipped to
  consumers via the `types` field in `package.json`.
- **Sphinx** documents the SWIG-generated `cec` module. The CI job runs
  `swig -python -doxygen`, which carries `cec.h`'s Doxygen comments into
  `cec.py` docstrings, then autodoc reads them with the native `_cec` extension
  mocked — so no full libCEC compile is needed just to build the docs.
- **rustdoc** reads the crate's own doc comments; there is no config file
  because the crate *is* the input. It needs no libCEC installed — documenting a
  crate never links it, and `build.rs` only emits link flags. The CI job runs
  with `RUSTDOCFLAGS=-D warnings`, so a broken intra-doc link fails the build.
  rustdoc writes its output under `libcec/` and generates no root index for a
  single crate, so [`rust/index.html`](rust/index.html) is copied in as a
  redirect.

## Building locally

Each generator runs independently; you only need the tool for the binding you
care about. Output is git-ignored (see [`.gitignore`](.gitignore)).

```sh
# C / C++  (needs: doxygen, graphviz)
cd docs/api/doxygen
git clone --depth 1 -b v2.3.4 https://github.com/jothepro/doxygen-awesome-css /tmp/dac
cp /tmp/dac/doxygen-awesome*.css .
doxygen Doxyfile              # -> docs/api/doxygen/html/

# .NET  (needs: .NET SDK 8+, `dotnet tool install -g docfx`)
cd docs/api/dotnet
docfx metadata docfx.json && docfx build docfx.json   # -> _site/

# Node.js  (needs: Node 18+)
cd docs/api/nodejs
npm install --no-save typedoc typescript @types/node
npx typedoc --options typedoc.json                    # -> _site/

# Python  (needs: swig 4+, python, pip install -r python/requirements.txt)
mkdir -p pygen
swig -c++ -python -doxygen -Iinclude -Isrc/libcec \
  -DCEC_LIB_VERSION_MAJOR=8 -outdir pygen -o pygen/cec_wrap.cxx src/libcec/libcec.i
CEC_PY_MODULE_DIR=$PWD/pygen sphinx-build -b html docs/api/python site/python

# Rust  (needs: a Rust toolchain; no libCEC required)
cd src/rust
cargo doc --no-deps --open                            # -> target/doc/libcec/
```

## Publishing

The workflow deploys to GitHub Pages via `actions/deploy-pages`. **One-time
setup:** in the repository's **Settings → Pages**, set *Source* to *GitHub
Actions*. After that, every qualifying push republishes automatically; it can
also be triggered manually from the Actions tab (`workflow_dispatch`).
