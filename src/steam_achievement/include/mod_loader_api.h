#pragma once

#ifdef _WIN32
#define MOD_LOADER_CALL __cdecl
#ifdef __cplusplus
extern "C" {
#endif

// message must be a null-terminated UTF-8 string.
typedef void (MOD_LOADER_CALL *mod_log_fn)(const char* message);
typedef void (MOD_LOADER_CALL *on_mod_load_fn)(mod_log_fn log);

// Every mod must export exactly this C symbol:
// __declspec(dllexport) void __cdecl on_mod_load(mod_log_fn log);

#ifdef __cplusplus
}
#endif
#endif
