# mod_loader

A generic x64 Windows mod loader. The system-DLL proxy mechanism is adapted from
UnityDoorstop, while the loader itself contains no Unity-specific behavior.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

The output is `mod_loader.dll`. At deployment time, rename it to a system DLL
imported by the target process: `version.dll`, `winhttp.dll`, or `dxgi.dll`.
It forwards calls to the real system DLL and then enumerates `mods/*.dll` under
the application directory.

## Mod ABI

A third-party mod only needs the released `include/mod_loader_api.h` header and
must export the following C symbol:

```cpp
#include "mod_loader_api.h"

extern "C" __declspec(dllexport)
void __cdecl on_mod_load(mod_log_fn log) {
    log(L"my mod loaded");
}
```

A DLL without `on_mod_load` is logged and unloaded. Loader messages are written
to `mod.log` in the application directory.

## Upstream source

The proxy implementation is derived from NeighTools/UnityDoorstop 4.5.0. See
[`UPSTREAM.md`](UPSTREAM.md) for the exact file mapping, local adaptations, and
the repeatable update procedure.
