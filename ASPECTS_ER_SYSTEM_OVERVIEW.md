# Elden Rim System Overview

This document is the current runtime contract for this repository.

## 1) Source of truth

- ER attributes and ER level are stored in SKSE serialization (`Persist::*`).
- Derived values are recomputed from serialized attributes/level and applied to player state.
- Vanilla XP progression can be disabled; ER progression becomes primary.

## 2) Core systems

- PrismaUI replaces the vanilla level-up interaction for ER allocation.
- Each confirmed attribute point increases ER level by 1 and consumes gold according to `leveling_curve.md`.
- Derived stats are applied on confirm and when the UI opens.
- Optional player-only `Actor::GetLevel` override returns ER level for quest/condition parity.

## 3) Data exposed by the plugin

- Current attributes: `vig`, `mnd`, `end`, `str`, `dex`, `intl`, `fth`, `arc`
- Current ER level
- Derived:
  - `maxHP`, `maxMP`, `maxSP`, `carryWeight`
  - defense sheet
  - threshold sheet
  - equip load sheet

Use:
- Internal snapshot API: `ER::GetCurrentStatsSnapshot()`
- Public C++ API: `RequestPluginAPI` (`ERLS_API.h`)
- Papyrus API: `ERLS.psc`

## 4) Perk-gated gameplay bridge

Because vanilla perk spending UI is bypassed, the plugin provides:

- Perk point parity from ER level delta (`EnablePerkPointParity`)
- Auto-unlock rules by ER level (`EnablePerkAutoUnlock`)
  - configured in `SKSE/Plugins/eldenrimlevelingsystem_perk_unlocks.json`
  - supports robust lookup via `modName + formId` for perk overhauls
  - supports `editorId` fallback

## 5) Integration boundary (external mods / frameworks)

This plugin owns:
- ER attribute state
- ER level progression
- Derived sheet computation

External systems (combat framework, alternate starts, perk overhauls) should:
- read values via public API or Papyrus API
- avoid writing direct internal globals
- call API setters when applying starter templates

## 6) Important files

- `skse_code/src/Prisma.cpp` — UI flow and state mutation
- `skse_code/src/DerivedStats.*` — derived formulas and application
- `skse_code/src/Serialization.*` — persistent model
- `skse_code/src/PerkProgression.*` — perk parity and auto-unlock
- `skse_code/src/ERLS_API.h` + `skse_code/src/plugin.cpp` — native plugin API
- `skse_code/src/PapyrusAPI.cpp` + `SKSE/Plugins/Source/ERLS.psc` — Papyrus API

