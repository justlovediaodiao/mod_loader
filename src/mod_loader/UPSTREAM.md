# UnityDoorstop upstream provenance

## Upstream

- Project: `NeighTools/UnityDoorstop`
- Repository: <https://github.com/NeighTools/UnityDoorstop>
- Local source snapshot: `UnityDoorstop-master/`
- Version reported by `info.lua`: `4.5.0`
- License: LGPL-2.1

The local snapshot does not contain Git metadata, so no upstream commit hash is
available. When refreshing it, record the new tag and commit hash in this file
before applying changes.

## Source mapping

| Upstream source | Local destination | Treatment |
| --- | --- | --- |
| `src/windows/proxy/proxylist.txt` | `proxylist.txt` | Copied verbatim. |
| `src/windows/proxy/proxy.h` | `proxy.cpp` (`load_proxy`) | Adapted to the Win32 API and local path storage; preserves adjacent `<name>_alt.dll` then System32 fallback order. |
| `src/windows/build_tools/proxygen.lua` | `tools/generate_proxy.py` | Reimplemented in Python with the same export generation model. |
| `src/windows/build_tools/proxy.c.in` | generated `proxy_generated.cpp` | Equivalent generated function-pointer table and forwarding functions. |
| `src/windows/build_tools/dll.def.in` | generated `proxy.def` | Equivalent export-name mapping. |

## Intentionally excluded upstream behavior

This component does not carry UnityDoorstop's Unity runtime bootstrap, Mono,
IL2CPP, CoreCLR, IAT hooks, command-line modification, `boot.config` override,
or managed assembly loading. Those features are unrelated to the generic native
mod loader.

## Local behavior added after the proxy stage

- Enumerate `<application directory>/mods/*.dll`.
- Require the C export `on_mod_load`.
- Pass a thread-safe `mod_log_fn` callback to each accepted mod.
- Inspect each DLL's PE export table and skip files that do not implement the
  mod ABI.
- Write loader diagnostics to `mod.log`.

## Updating from UnityDoorstop

1. Replace or refresh the upstream snapshot and record its tag and commit above.
2. Diff the new `src/windows/proxy/proxylist.txt` against local `proxylist.txt`.
3. Review changes to upstream `proxy.h`, especially DLL name handling, adjacent
   `_alt.dll` lookup, System32 lookup, and failure behavior.
4. Review `proxygen.lua`, `proxy.c.in`, and `dll.def.in` for generator changes.
5. Apply relevant changes only to the mapped local files. Do not import runtime
   bootstrap code into this component.
6. Run `python tools/generate_proxy.py`.
7. Verify that every non-empty `proxylist.txt` line has one generated pointer,
   one `GetProcAddress` assignment, one forwarding function, and one DEF export.
8. Build x64 and test all supported deployment names: `version.dll`,
   `winhttp.dll`, and `dxgi.dll`.
9. Confirm that a DLL with `on_mod_load` receives the logging callback and that
   a DLL without the symbol is skipped without running `DllMain`.
