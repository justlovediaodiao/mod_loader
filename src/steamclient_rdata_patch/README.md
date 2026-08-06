# steamclient_rdata_patch

A standalone Windows x64 mod loaded by `mod_loader`. During mod initialization,
it synchronously installs a `LoadLibraryExW` detour. When the original function
successfully loads `steamclient64.dll`, the mod changes the loaded image's
`.rdata` section to `PAGE_READWRITE` in the current process before returning the
module handle to the caller.

The requested path is never changed. The Steam installation remains untouched,
and no local copy of `steamclient64.dll` is required.

The detour uses the pinned `MinHook v1.3.4`
release, which CMake fetches during the GitHub Actions build.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

You can also run the repository's GitHub Actions workflow. Its artifact is named
`steamclient_rdata_patch-windows-x64`.

## Install

Install `mod_loader` first, then arrange the files as follows:

```text
<game executable directory>/
  mods/
    steamclient_rdata_patch.dll
```

Remove the old local `steamclient64.dll` from the game executable directory so
that Steam can load its original absolute path. After a successful installation,
`mod.log` should contain:

```text
steamclient_rdata_patch: installed LoadLibraryExW post-load patch
steamclient_rdata_patch: made steamclient64.dll .rdata writable in this process
```

The hook calls the original loader first and does not use a polling thread. The
caller cannot continue past `LoadLibraryExW` until the in-process protection
change has completed.
