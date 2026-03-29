# Project Context: Elden Rim Leveling System

This repository contains a native SKSE mod that replaces vanilla level-up flow with an Elden Ring-style attribute progression loop.

## Current architecture

- **Core plugin**: `skse_code/` (`eras.dll`)
- **UI**: PrismaUI + React app in `skse_code/ui/`
- **State storage**: SKSE serialization (`Persist::*`) for:
  - ER level
  - ER attributes (8 stats)
  - progression sync state (perk parity baseline)
- **No ActorValueGenerator dependency** for ER stat storage
  - ER attributes are no longer dependent on AVG alias slots
  - derived sheet values are computed on demand and exposed via APIs

## Runtime systems implemented

- Prisma-based replacement for level/attribute allocation UI
- ER rune/gold cost formula for attribute increases
- Derived stats recomputation and application to player (HP/MP/SP/CarryWeight + sheet values)
- Vanilla XP and vanilla level advancement blocking (configurable)
- Player-only `GetLevel` override option for ER level parity with quest checks
- Kill-gold reward system with external JSON tuning
- Perk progression bridge:
  - perk point parity from ER level delta
  - auto-unlock by ER level from config rules
- Public APIs:
  - C++ plugin API (`RequestPluginAPI`, `ERAS_API.h`)
  - Papyrus native API (`ERAS.psc`)

## Integration files to package

- `SKSE/Plugins/eras.ini`
- `SKSE/Plugins/eras_gold_kill.json`
- `SKSE/Plugins/eras_perk_unlocks.json`
- `SKSE/Plugins/Source/ERAS.psc` (for external Papyrus mods)
- PrismaUI built assets (see `skse_code/README_SKSE.md`)

## Quick map: where to change what

- ER level/gold progression: `skse_code/src/Economy.cpp`, `leveling_curve.md`
- Attribute allocation flow/UI bridge: `skse_code/src/Prisma.cpp`
- Derived calculations and snapshots: `skse_code/src/DerivedStats.*`
- Serialization model: `skse_code/src/Serialization.*`
- Config parsing: `skse_code/src/Config.*`
- Perk parity/auto-unlock: `skse_code/src/PerkProgression.*`
- Public C++ API: `skse_code/src/ERAS_API.h`, `skse_code/src/plugin.cpp`
- Papyrus API: `skse_code/src/PapyrusAPI.cpp`, `SKSE/Plugins/Source/ERAS.psc`
