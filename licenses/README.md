# Licence sources

Every distribution ships a different set of third-party code, so each needs a
licence text that lists what is actually in it:

| Distribution | Third-party it carries | Output |
| --- | --- | --- |
| source tree | AMD ADL headers | `LICENSE.md` |
| Debian packages | AMD ADL headers, Debian packaging | `debian/copyright` |
| Windows installer | AMD ADL headers, libusb-win32, Microsoft DPInst, node-addon-api | `build\LICENSE.md`, written by `windows/create-installer.py` |
| npm package | — | `src/nodejs/LICENSE.md` |
| crates.io package | — | `src/rust/LICENSE.md` |

Those outputs are assembled by `support/generate-licenses.py` from the files
here, so no licence text is maintained in two places:

- `components.json` — what each component is, who holds its copyright, which
  licence it is under, and which distributions carry it. **This is the file to
  edit.**
- `libcec.txt` — libCEC's own terms. The copyright years live in
  `components.json`, not here.
- `texts/` — the full licence texts, one per licence, shared by every
  component under it.

## Adding a component

Add it to `components.json` with the distributions that ship it, then
regenerate:

```
python3 support/generate-licenses.py
```

A component only some builds carry (`"optional": true`) is left out unless the
build asks for it by id — `--with node-addon-api`, which `create-installer.py`
passes when the Node.js addon is staged.

If the licence is one no component used before, drop its full text in `texts/`
and add it to the `licenses` and `texts` maps.

`--check` verifies the tracked outputs still match the manifest; it runs as
part of `support/release.py`, which refuses to cut a release if they have
drifted.
