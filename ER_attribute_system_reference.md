# ER Attribute System Reference

This document describes the current attribute model used by the plugin.

## Core attributes

- `VIG` (`vig`)
- `MND` (`mnd`)
- `END` (`end`)
- `STR` (`str`)
- `DEX` (`dex`)
- `INT` (`intl`)
- `FTH` (`fth`)
- `ARC` (`arc`)

All values are persisted in SKSE save data through `Persist::GetAttributes()` / `Persist::SetAttributes()`.

## Progression model

- ER level is stored separately from vanilla level.
- Confirmed attribute increases increase ER level by the same amount.
- Level cost is computed from `leveling_curve.md` (`ER::GoldCostToLevelUp`).

## Derived values

Derived stats are computed from attributes + ER level and exposed via snapshot:

- `maxHP`
- `maxMP`
- `maxSP`
- `carryWeight`
- defenses (`physical`, `magic`, `fire`, `lightning`, `frost`, `poison`)
- thresholds (`immunity`, `robustness`, `focus`, `vitality`, `madness`)
- equip load tiers (`max`, `light`, `medium`, `heavy`)

Primary code:
- `skse_code/src/DerivedStats.cpp`
- `skse_code/src/DerivedStats.h`

## Runtime application

Derived values are applied at least on:

- allocation confirmation in Prisma UI
- menu-open refresh path
- explicit API calls that request immediate apply (`applyNow=true`)

## External integration

Use one of:

- C++ plugin API: `ERLS_API.h` + exported `RequestPluginAPI`
- Papyrus API: `SKSE/Plugins/Source/ERLS.psc`

for starter templates, class initialization, and external mod sync.