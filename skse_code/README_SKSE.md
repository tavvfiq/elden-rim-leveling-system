# Elden Rim Leveling System (SKSE)

## Overview

`skse_code/` contains the native SKSE plugin and PrismaUI bridge for ER-style progression.

Key behavior:

- intercepts vanilla level/stat menus and shows Prisma UI
- stores ER level + attributes in SKSE serialization
- computes/applies derived stats
- supports gold-on-kill rewards
- supports perk point parity and perk auto-unlock
- exposes C++ and Papyrus APIs for external mods

## Build

- Platform: Windows
- Build system: `xmake`
- Project file: `skse_code/xmake.lua`
- Dependency: CommonLibSSE-NG path is set in `xmake.lua`

## Runtime package layout

Package these with your mod:

- `Data/SKSE/Plugins/eldenrimlevelingsystem.dll`
- `Data/SKSE/Plugins/eldenrimlevelingsystem.ini`
- `Data/SKSE/Plugins/eldenrimlevelingsystem_gold_kill.json`
- `Data/SKSE/Plugins/eldenrimlevelingsystem_perk_unlocks.json`
- `Data/SKSE/Plugins/Source/ERLS.psc` (for Papyrus consumers)

## PrismaUI packaging

- View path used by plugin: `EldenRimLevelingSystem/index.html`
- Build UI from `skse_code/ui/` and package output under PrismaUI views so that path resolves at runtime.

## Config summary

From `eldenrimlevelingsystem.ini`:

- `OverridePlayerGetLevel`
- `DisableVanillaXPGain`
- `EnableGoldKillDrops`
- `EnablePerkPointParity`
- `EnablePerkAutoUnlock`
- Equip load tuning (`LightFraction`, `MediumFraction`)

## Public APIs

### C++ plugin API

- Header: `skse_code/src/ERLS_API.h`
- Export: `RequestPluginAPI`
- Returns interface with getters for:
  - full player snapshot
  - ER level
  - attributes

### Papyrus API

- Script: `SKSE/Plugins/Source/ERLS.psc`
- Native functions:
  - `SetERLevel`
  - `SetAttributes`
  - `SetAttributesAndLevel`
  - `ApplyNow`

Use Papyrus API for alternate start/class mods to set starter templates.

