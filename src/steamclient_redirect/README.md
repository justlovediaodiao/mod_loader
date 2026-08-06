# steamclient_redirect

A standalone Windows x64 mod loaded by `mod_loader`. During mod initialization,
it synchronously installs a `LoadLibraryExW` detour. When that function is
asked to load `steamclient64.dll`, the request is redirected to the local copy
beside the game executable.

The detour uses the pinned `MinHook v1.3.4`
release, which CMake fetches during the GitHub Actions build.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

You can also run the repository's GitHub Actions workflow. Its artifact is named
`steamclient_redirect-windows-x64`.

## Install

Install `mod_loader` first, then arrange the files as follows:

```text
<game executable directory>/
  steamclient64.dll
  mods/
    steamclient_redirect.dll
```

`steamclient64.dll` is the local copy that should take precedence. After a
successful installation, `mod.log` should contain:

```text
steamclient_redirect: installed LoadLibraryExW detour
steamclient_redirect: redirecting steamclient64.dll to the local copy
```

The detour installation message should appear immediately after
`Loading mod: steamclient_redirect.dll`; it no longer waits for
`steam_api64.dll`. If `steamclient64.dll was already loaded` is reported at
this point, even `mod_loader`'s own mod initialization occurred too late.
