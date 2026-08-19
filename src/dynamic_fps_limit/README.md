# dynamic_fps_limit

A generic native `mod_loader` mod for games using NVIDIA Streamline DLSS-G.

- NVIDIA frame generation disabled: configurable, 60 FPS by default
- NVIDIA frame generation enabled: configurable, 120 FPS by default

The mod hooks the game's `slDLSSGSetOptions` call and updates either the NVIDIA
Reflex frame limit or the RTSS limit for the current game profile whenever
DLSS-G is enabled or disabled. It is not tied to any particular game and should
work with any title that loads `sl.interposer.dll` and exposes the standard
Streamline APIs.

Configure the mod in the `config.ini` next to `dynamic_fps_limit.dll`:

```ini
[dynamic_fps_limit]
frame_generation_off_fps=60
frame_generation_on_fps=120
limiter=reflex
```

- `limiter`: `reflex` (default) or `rtss`.

## Requirements

- [mod_loader](https://github.com/justlovediaodiao/mod_loader)
- RivaTuner Statistics Server (RTSS) running while the game is running when
  `limiter=rtss`
- An NVIDIA GPU supported by the game's DLSS Frame Generation implementation

## Installation

Extract the artifact into the game's application directory, i.e. the same
directory where the mod_loader proxy DLL is installed.

The resulting layout must be:

```text
GameDirectory/
|-- mods/
|   |-- dynamic_fps_limit.dll
|   `-- config.ini
```

For a typical Unreal Engine game, this is often:

```text
<Game>/<Game>Game/Binaries/Win64/
```

## RTSS profile permissions

This section only applies when `limiter=rtss`. The game process must be able to
update its RTSS profile. Ensure that your Windows user has **Modify** permission
for:

```text
C:\Program Files (x86)\RivaTuner Statistics Server\Profiles
```

You can set this through **Properties > Security > Edit**, then grant your user
or the `Users` group **Modify** permission. If the game profile already exists,
also verify the permissions of its `.cfg` file. Without write permission, the
mod may load normally while the frame limit never changes.

## Building

Use Visual Studio with the C++ workload and CMake:

```text
cmake -S . -B build -A x64
cmake --build build --config Release
```

Output:

```text
build/Release/dynamic_fps_limit.dll
```

Copy both `dynamic_fps_limit.dll` and `config.ini` into the game's `mods`
directory, or use `cmake --install` to install both files together.
