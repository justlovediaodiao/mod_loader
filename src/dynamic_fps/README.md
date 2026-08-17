# dynamic_fps

A generic native `mod_loader` mod for games using NVIDIA Streamline DLSS-G.

- NVIDIA frame generation inactive: configurable, 60 FPS by default
- NVIDIA frame generation actively presenting generated frames: configurable,
  120 FPS by default

The mod queries the game's NVIDIA Streamline DLSS-G state and checks the number
of frames actually presented. It updates the RTSS limit for the current game
profile based on whether generated frames are really being inserted. It is not
tied to any particular game and should work with any title that loads
`sl.interposer.dll` and exposes the standard Streamline DLSS-G API.

Configure the two limits in the `config.ini` next to `dynamic_fps.dll`:

```ini
dynamic_fps:
frame_generation_off_fps=60
frame_generation_on_fps=120
```

## Requirements

- [mod_loader](https://github.com/justlovediaodiao/mod_loader)
- RivaTuner Statistics Server (RTSS) running while the game is running
- An NVIDIA GPU supported by the game's DLSS Frame Generation implementation

## Installation

Extract the artifact into the game's application directory, i.e. the same
directory where the mod_loader proxy DLL is installed.

The resulting layout must be:

```text
GameDirectory/
|-- mods/
|   |-- dynamic_fps.dll
|   `-- config.ini
```

For a typical Unreal Engine game, this is often:

```text
<Game>/<Game>Game/Binaries/Win64/
```

## RTSS profile permissions

The game process must be able to update its RTSS profile. Ensure that your
Windows user has **Modify** permission for:

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
build/Release/dynamic_fps.dll
```

Copy both `dynamic_fps.dll` and `config.ini` into the game's `mods` directory,
or use `cmake --install` to install both files together.
