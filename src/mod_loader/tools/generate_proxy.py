#!/usr/bin/env python3
"""Generate forwarding source/DEF using UnityDoorstop's proxygen layout."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
names = [x.strip() for x in (ROOT / "proxylist.txt").read_text().splitlines() if x.strip()]

defs = "\n".join(f"static FARPROC __{n}__;" for n in names)
loads = "\n".join(f'    __{n}__ = GetProcAddress(dll, "{n}");' for n in names)
funcs = "\n".join(
    f'extern "C" intptr_t exp_{n}() {{ return ((intptr_t (WINAPI*)())__{n}__)(); }}'
    for n in names
)
source = f'''// Generated from proxylist.txt; do not edit.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

{defs}

extern "C" void load_functions(HMODULE dll) {{
{loads}
}}

{funcs}
'''
exports = "\n".join(f" {n}=exp_{n}" for n in names)
(ROOT / "proxy_generated.cpp").write_text(source, newline="\n")
(ROOT / "proxy.def").write_text(f"LIBRARY mod_loader\nEXPORTS\n{exports}\n", newline="\n")
print(f"generated {len(names)} forwarding exports")
