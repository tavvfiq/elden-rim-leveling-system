# AspectsAttributes (SKSE) notes

## What’s in this folder

`skse_code/` contains the native SKSE plugin (CommonLibSSE-NG + PrismaUI) that:

- redirects the vanilla `LevelUpMenu` to a PrismaUI-based ER attribute allocation screen
- reads/writes ER attribute actor values (`ER_*`) and recomputes derived stats

## Plugin INI (replaces MCM globals for tuning)

Tuning is read from **`Data/SKSE/Plugins/<DLL_stem>.ini`** next to the built DLL (for the default xmake target, `eldenrimlevelingsystem.dll` → `eldenrimlevelingsystem.ini`).

- Shipped template: `SKSE/Plugins/eldenrimlevelingsystem.ini` in this repo (copy into your mod’s `Data/SKSE/Plugins/` when packaging).
- **`[EquipLoad]`** — `LightFraction` / `MediumFraction` drive `ER_EQUIPLOAD_*_AVG` tier cutoffs.
- **`[Tuning]`** — numeric defaults copied from the old MCM Helper sliders (removed from this repo); not all keys are wired into native derived formulas yet, but they load for future use.

If your **ESP** still registers legacy MCM / Papyrus quests, clean those references in CK/xEdit when you ship SKSE-only.

## Build

This plugin is intended to be built on **Windows**.

- **Build system**: `xmake` (see `skse_code/xmake.lua`)
- **CommonLibSSE-NG**: referenced via an absolute path in `xmake.lua` (adjust on the Windows machine)

## PrismaUI

- C++ side expects the Prisma view at: `AspectsAttributes/index.html`
- UI sources live at: `skse_code/ui/` (Vite + React)
- You need to package the built UI into:
  - `Data/PrismaUI/views/AspectsAttributes/` (so the path `AspectsAttributes/index.html` resolves)

## Actor Values (ER)

The native plugin expects these custom actor values to exist:

- `ER_VIG`, `ER_MND`, `ER_END`, `ER_STR`, `ER_DEX`, `ER_INT`, `ER_FTH`, `ER_ARC`

ActorValueData config was updated at:

- `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`

### Important

Updating the TOML **does not automatically add the AVs** to your plugin/AVG setup. You still need to generate/register these AVs using your Actor Value Generator workflow and ensure they are available at runtime.

