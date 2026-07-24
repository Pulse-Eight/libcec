### Debian
Use the following commands to create a Debian package:
```
sudo apt-get build-dep . # to install build dependencies
sed "s/#DIST#/$(lsb_release -cs)/g" debian/changelog.in > debian/changelog
dpkg-buildpackage --no-sign # omit this argument if you have a private key to sign with
```

This builds the following binary packages:
* `libcec8` — the shared library (named after the SONAME; supersedes the old `libcec4`-`libcec7` names)
* `libcec8-dev` — headers, the pkg-config and CMake package config, and the static library
* `cec-utils` — the `cec-client` and `cecc-client` example clients
* `python-libcec` — the Python 3 bindings
* `libcec-dotnet` — the cross-platform `LibCecSharp` .NET binding and its NuGet package
* `node-libcec` — the native Node.js (N-API) binding, installed as a global node module
* `libcec` — a meta package pulling in the shared library

The `libcec-dotnet` package requires the .NET 8 SDK (`dotnet-sdk-8.0`) at build time
and the .NET 8 runtime at install time; the build enables it via `-DENABLE_DOTNET_LIB=1`
(see [debian/rules](../debian/rules)). The build also enables the Linux kernel CEC
framework, Exynos and AOCEC backends.

The `node-libcec` package is built with `node-gyp` (build-deps: `nodejs`,
`libnode-dev`, `node-gyp`, `node-addon-api`) rather than cmake's `ENABLE_NODE_LIB`
option, which uses `npm` and would need network access. `debian/rules` builds the
addon against the freshly-staged libCEC via `PKG_CONFIG_SYSROOT_DIR` and installs
it as a global node module under `/usr/lib/<triplet>/nodejs/libcec`.
