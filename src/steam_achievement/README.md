# steam_achievement

A Windows x64 Steam achievement mod using the `src/mod_loader` ABI, with no
Steamworks SDK link dependency.

This mod is designed to recover stuck achievements when you have already met
the in-game requirements but the game failed to unlock them on Steam. It is
not intended to unlock arbitrary or unearned achievements. The mod cannot
verify in-game progress; only configure achievements you have already earned.

## Build

Run from this mod's directory using Visual Studio with the C++ workload and CMake:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
cmake --install build --config Release --prefix out
```

## Usage

Install mod_loader first, then place `steam_achievement.dll` in the game's
`mods` directory. Add the following section to `config.ini` next to the mod
DLL. If the file already exists, merge this section into it.

```ini
[steam_achievement]
list_achievement=1
achievement_id=
```

Launch the game and check `mod.log` in the game directory for achievement IDs,
titles, and descriptions. Titles and descriptions use the language returned
by Steam; hidden achievements may have empty descriptions.

Identify the stuck achievements whose requirements you have already met,
enter their IDs in the configuration, and restart the game. For example:

```ini
[steam_achievement]
list_achievement=0
achievement_id=ACH_WIN_ONE_GAME, ACH_FINISH_TUTORIAL
```

Replace the example IDs with actual IDs from your game; IDs are case-sensitive.
Separate multiple IDs with commas. Leading and trailing whitespace, empty
entries, and duplicate IDs are ignored.

`list_achievement` accepts `0`/`1` or `false`/`true` and defaults to disabled.
`achievement_id` defaults to empty. Both settings can be enabled together;
the mod lists achievements before attempting recovery. Configuration is read
at load time, and the operation runs once per launch. After confirming recovery,
clear `achievement_id` to stop requesting it on subsequent launches.

ASCII IDs can be saved directly. For non-ASCII characters, save the configuration
as UTF-16 LE with a BOM because it uses the Windows INI API. Logs use UTF-8.

## Behavior and compatibility

A background worker waits up to 60 seconds for the game's loaded
`steam_api64.dll`, then up to another 60 seconds for achievement data.
It uses `GetProcAddress` to resolve a
SteamUserStats v013/v012/v011 accessor and the achievement-related flat API
exports. The game must initialize Steam itself. Older or customized DLLs that
lack required exports are unsupported, and missing exports are logged. The mod
does not load Steam DLLs itself or initialize or shut down Steam.

For older SDKs, the mod requests data through `RequestCurrentStats` when needed
and available, retrying at most once every 10 seconds while data remains
unavailable. Request acceptance does not confirm asynchronous completion.
Request results and changes in the reported count are logged.
A successful read of a known achievement indicates that data is
ready; the game continues to process Steam callbacks. If Steam keeps reporting
zero achievements, the mod logs `count=0` after the wait. This can mean that the
game has no achievements or that its data is unavailable.

If this happens for a game that has Steam achievements, check that Steam is
logged in and launch the game through Steam. Verify that the game's App ID is
correct. Include all `steam_achievement:` lines from `mod.log` when reporting
the problem, especially the selected accessor and `RequestCurrentStats`
results. Retries cannot fix an incorrect App ID or missing achievement data.

Invalid IDs, already unlocked achievements, and API failures are logged per
entry. After successful `SetAchievement` calls, the mod calls `StoreStats` once
to submit the changes. A successful return only means that submission was
accepted; the mod does not listen for server confirmation callbacks. Verify
the final achievement state in Steam. See the
[Steamworks ISteamUserStats documentation](https://partner.steamgames.com/doc/api/ISteamUserStats).

Building and in-game behavior still need to be verified on Windows x64.
