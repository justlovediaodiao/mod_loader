# Mod Loader

This repository contains three components that can be built and released independently:

- [`src/mod_loader`](src/mod_loader): a generic Windows mod loader and its mod ABI.
- [`src/unity_fps_limit`](src/unity_fps_limit): a Unity FPS/VSync mod using that ABI.
- [`src/steamclient_rdata_patch`](src/steamclient_rdata_patch): a targeted
  loader patch for a writable-section Steam client copy kept in the Steam
  installation directory.

The components intentionally keep separate CMake projects and release
packages. GitHub Actions builds them independently on Windows x64 and uploads
one artifact per component. See each component's README for local build commands.
