# steamclient_rdata_patch

A standalone Windows x64 mod loaded by `mod_loader`. It reads the installed
Steam client path from:

```text
HKCU\Software\Valve\Steam\ActiveProcess\SteamClientDll64
```

It then installs a `LoadLibraryExW` detour. Requests for
`steamclient64.dll` are redirected to `steamclient6p.dll` in that same
Steam installation directory. After the patched DLL loads, its loader entry's
full path and base name are changed back to those of the original
`steamclient64.dll`. No file in the game directory is used.

The detour uses the pinned `MinHook v1.3.4` release, which CMake fetches during
the GitHub Actions build.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

The GitHub Actions artifact is named
`steamclient_rdata_patch-windows-x64`.

## Install

Keep the original Steam file unchanged and put the writable-section copy beside
it under a different name:

```text
C:\Program Files (x86)\Steam\
  steamclient64.dll
  steamclient6p.dll
```

Install mod loader first. Install the mod as:

```text
<game executable directory>\
  mods\
    steamclient_rdata_patch.dll
```
