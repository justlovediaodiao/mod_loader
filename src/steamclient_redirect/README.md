# steamclient_redirect

A standalone Windows x64 mod loaded through `mod_loader`. It modifies only the `LoadLibraryExW` entry in the import table of `steam_api64.dll`: when the function is asked to load `steamclient64.dll`, it loads a local copy from the same directory as the game executable instead.

It does not modify `steamclient64.dll`, alter any virtual tables, or hook other modules or DLL-loading APIs.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

Alternatively, run the repository’s GitHub Actions workflow. The artifact is named
`steamclient_redirect-windows-x64`.

## Install

Install `mod_loader` first, then place the files in the following structure:

```text
<game executable directory>/
  steamclient64.dll
  mods/
    steamclient_redirect.dll
```

`steamclient64.dll` is the local copy that should be loaded with priority. After a successful installation, `mod.log` should contain:

```text
steamclient_redirect: installed LoadLibraryExW IAT redirect
steamclient_redirect: redirecting steamclient64.dll to the local copy
```

If the log reports `steamclient64.dll was already loaded`, the mod was loaded too late, and the existing module can no longer be replaced through load redirection for the current process.
