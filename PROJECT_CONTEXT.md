# Project context: Aspects / Elden Rim Leveling System

This workspace is a **Skyrim mod Data-style layout**: SKSE plugin sources, PrismaUI bundle, ActorValueData TOML, plugin INI, and markdown specs. **Papyrus sources and MCM Helper JSON were removed** from the repo; tuning lives in the SKSE plugin INI.

## What this mod is (current direction)

- **Native SKSE plugin** (`skse_code/`, xmake target `eldenrimlevelingsystem`): hooks vanilla `LevelUpMenu`, drives **PrismaUI** for ER-style allocation, gold-as-runes level cost, `ER_*` attributes, derived stats, and published `ER_*_AVG` sheet values for ERCF.
- **ESP** (`AspectsAttributes.esp` when you maintain it in the CK): still needed in-game for forms / AVG registration workflow; it is **not** tracked in this workspace snapshot in some setups.
- **Tuning**: `SKSE/Plugins/eldenrimlevelingsystem.ini` (must sit next to the built DLL; same stem as the DLL file name).

## Repository layout (top-level)

- `skse_code/`
  - C++ plugin (CommonLibSSE-NG), Prisma bridge, hooks, serialization, economy, derived stats, config loader.
- `skse_code/ui/`
  - Vite + React PrismaUI app; build output is packaged under PrismaUI views (see `skse_code/README_SKSE.md`).
- `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`
  - Declares `MM_*` legacy slots (if still used) plus `ER_*` and published `ER_*_AVG` adaptive AVs for `AspectsAttributes.esp`.
- `SKSE/Plugins/eldenrimlevelingsystem.ini`
  - Shipped template for plugin settings (equip-load fractions, reserved tuning keys).
- `*.md`
  - Design and system docs (`ASPECTS_ER_SYSTEM_OVERVIEW.md`, `ER_ATTRIBUTE_REBUILD_SPEC.md`, `derived_stats_map.md`, etc.).

## Quick “where do I change X?” map

- **Level-up gold curve**: `leveling_curve.md`, `skse_code/src/Economy.cpp`
- **ER attributes + allocation UI**: `skse_code/src/Prisma.cpp`, `skse_code/ui/src/App.tsx`
- **Derived stats + published `*_AVG` AVs**: `skse_code/src/DerivedStats.*`
- **INI tuning (equip load tiers; future HP/MP tuning keys)**: `SKSE/Plugins/eldenrimlevelingsystem.ini`, `skse_code/src/Config.*`
- **Custom AV definitions for AVG**: `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`
