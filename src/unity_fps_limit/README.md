# unity_fps_limit

An independently released Unity FPS/VSync mod supporting both IL2CPP and Mono.
It uses the mod_loader `on_mod_load` ABI. Install mod loader first.

## Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

Copy both released files into the game's `mods` directory:

```text
mods/
  unity_fps_limit.dll
  config.ini
```

`config.ini`:

```ini
[unity_fps_limit]
fps=-1
vsync=0
```

`fps` is parsed as a signed 32-bit integer. `-1` is supported and passed
unchanged to `UnityEngine.Application.set_targetFrameRate`. `vsync` is also a
signed integer and is passed to `UnityEngine.QualitySettings.set_vSyncCount`.

The mod automatically detects `GameAssembly.dll`, `mono-2.0-bdwgc.dll`, or
`mono.dll` and uses the corresponding IL2CPP or Mono runtime API to invoke:

```text
UnityEngine.QualitySettings.set_vSyncCount(vsync)
UnityEngine.Application.set_targetFrameRate(fps)
```