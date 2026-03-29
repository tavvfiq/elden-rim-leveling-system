# ER Attribute Rebuild Spec (Current State)

This file records what is implemented today after the rebuild and integration passes.

## Implemented goals

- ER attributes and ER level are persisted in SKSE serialization.
- PrismaUI is the primary allocation UI.
- Attribute confirm applies:
  - attribute deltas
  - ER level increment parity
  - derived stat recompute/apply
- Vanilla XP and vanilla level progression can be disabled by config.
- Player `GetLevel()` can be overridden to return ER level (player only).
- Perk-gated systems are bridged by:
  - perk point parity
  - perk auto-unlock rules

## APIs for external integration

### Native C++ API

- Export: `RequestPluginAPI`
- Header: `skse_code/src/ERAS_API.h`
- Provides snapshot/level/attributes getters.

### Papyrus API

- Script: `SKSE/Plugins/Source/ERAS.psc`
- Native functions:
  - `SetERLevel`
  - `SetAttributes`
  - `SetAttributesAndLevel`
  - `ApplyNow`

Use this for alternate-start starter templates and class systems.

## Config files

- `SKSE/Plugins/eras.ini`
  - progression and runtime toggles
- `SKSE/Plugins/eras_gold_kill.json`
  - kill-gold formula tuning
- `SKSE/Plugins/eras_perk_unlocks.json`
  - perk unlock rules by ER level (`modName+formId` or `editorId`)

## Remaining extension points

- More Papyrus convenience calls (preset-by-name loader).
- Additional triggers for forced derived recompute if needed by external mods.
- Optional export of write APIs in native C++ interface if third-party DLLs need direct mutation.

