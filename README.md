# Mod Loader

This repository contains some components that can be built and released independently:

- [`mod_loader`](src/mod_loader): a generic Windows mod loader and its mod ABI.
- [`dynamic_fps`](src/dynamic_fps): a generic Streamline DLSS-G/RTSS dynamic FPS
  mod using that ABI.
- [`unity_fps_limit`](src/unity_fps_limit): a Unity FPS/VSync mod using that ABI.
- [`steamclient_rdata_patch`](src/steamclient_rdata_patch): a targeted
  loader patch for a writable-section Steam client copy kept in the Steam
  installation directory.
- [`rdata_patch`](src/rdata_patch): a tool to modify exe/dll rdata to writeable.

The components intentionally keep separate CMake projects and release
packages. GitHub Actions builds them independently on Windows x64 and uploads
one artifact per component. See each component's README for local build commands.
