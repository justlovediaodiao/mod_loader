# reflex_fps_limit

A generic native `mod_loader` mod that enforces an NVIDIA Reflex frame limit
in games using NVIDIA Streamline.

The mod hooks the game's `slReflexSetOptions` call and replaces its frame limit
with the configured value. It retries initialization until the game loads
`sl.interposer.dll` and exposes the Reflex API, so it is not tied to a
particular title.

Configure the mod in the `config.ini` next to `reflex_fps_limit.dll`:

```ini
[reflex_fps_limit]
fps=60
```

- `fps`: the frame limit to enforce. The default is `60`; use `0` to disable
  the Reflex frame limit.

## Requirements

- [mod_loader](https://github.com/justlovediaodiao/mod_loader)
- A game that uses NVIDIA Streamline and exposes the Reflex API through
  `sl.interposer.dll`
- An NVIDIA GPU supported by the game's Reflex implementation

## Installation

Extract the artifact into the game's application directory, i.e. the same
directory where the mod_loader proxy DLL is installed.

The resulting layout must be:

```text
GameDirectory/
|-- mods/
|   |-- reflex_fps_limit.dll
|   `-- config.ini
```

For a typical Unreal Engine game, this is often:

```text
<Game>/<Game>Game/Binaries/Win64/
```

## Building

Use Visual Studio with the C++ workload and CMake:

```text
cmake -S . -B build -A x64
cmake --build build --config Release
```

Output:

```text
build/Release/reflex_fps_limit.dll
```

Copy both `reflex_fps_limit.dll` and `config.ini` into the game's `mods`
directory, or use `cmake --install` to install both files together.
