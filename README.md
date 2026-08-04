# Mod Loader

This repository contains two components that can be built and released independently:

- [`src/mod_loader`](src/mod_loader): a generic Windows mod loader and its mod ABI.
- [`src/unity_fps_limit`](src/unity_fps_limit): a Unity FPS/VSync mod using that ABI.

The two components intentionally keep separate CMake projects and release
packages. GitHub Actions builds them independently on Windows x64 and uploads
`mod_loader-windows-x64` and `unity_fps_limit-windows-x64` as separate workflow
artifacts. See each component's README for local build commands.
