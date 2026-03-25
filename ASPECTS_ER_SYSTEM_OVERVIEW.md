# Aspects ER system overview (contract for ERCF)

This document describes the **overall runtime system** implemented in this repo (`Aspects`) for:

- ER-style **level** (separate from Skyrim level)
- ER-style **attributes** (8 stats)
- **Derived stats** (HP/MP/SP/CarryWeight)
- Published **computed “AVG” ActorValues** intended as the **lookup contract** for `elden-rim-combat-framework` (ERCF)

Scope boundary:

- This repo is the **actor sheet source of truth** (attributes + computed sheet numbers).
- ERCF owns **on-hit processing**, damage splitting, layer-2 mitigation, and status application logic.

---

## 1) Data model (what exists at runtime)

### 1.1 ER attributes (custom ActorValues)

These 8 custom AVs are used as the canonical attribute storage:

- `ER_VIG` (Vigor)
- `ER_MND` (Mind)
- `ER_END` (Endurance)
- `ER_STR` (Strength)
- `ER_DEX` (Dexterity)
- `ER_INT` (Intelligence)
- `ER_FTH` (Faith)
- `ER_ARC` (Arcane)

Implementation:

- Declared for ActorValueData in `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`
- Resolved at runtime using `ActorValueList::LookupActorValueByName(...)` (see `skse_code/src/Attributes.*`)

### 1.2 ER level + points (stored via SKSE serialization)

This repo tracks a **separate ER level** and attribute points:

- **ERLevel**: stored via `Persist::GetERLevel()` / `Persist::SetERLevel()`
- **Unspent points**: stored via `Persist::GetUnspentPoints()` / `Persist::SetUnspentPoints()`
- **Pending allocation**: transient (not serialized); used only while the PrismaUI allocation screen is open

Notes:

- This is intentionally **decoupled from** `PlayerRef.GetLevel()` / Skyrim XP.
- ER level-up cost uses the formula in `leveling_curve.md` (implemented in `skse_code/src/Economy.cpp`).

---

## 2) Allocation & UI flow (PrismaUI LevelUpMenu replacement)

This repo hooks the vanilla `LevelUpMenu` and shows a PrismaUI view instead.

High-level flow:

- **Level-up purchase**
  - UI requests `levelUp`
  - Plugin checks gold and spends it (`TrySpendPlayerGold`)
  - Increments `ERLevel` and `UnspentPoints`
- **Allocation**
  - UI requests `allocatePoint` / `refundPoint`
  - Plugin moves points between `UnspentPoints` and a **pending delta**
- **Confirm**
  - UI requests `confirmAllocation`
  - Plugin writes the pending delta into the 8 `ER_*` AVs
  - Plugin recomputes derived stats and applies them
  - Plugin publishes computed “AVG” AVs (see section 4)
- **Cancel**
  - UI requests `cancelAllocation`
  - Plugin refunds all pending points and closes UI

Relevant code:

- `skse_code/src/Prisma.cpp` (UI bridge + state JSON + allocation events)
- `skse_code/src/Economy.cpp` (gold and level-up cost)
- `skse_code/src/Attributes.*` (read/write ER AVs)

---

## 3) Derived stats (vanilla AVs we set)

The plugin computes and applies these **vanilla** AV base values:

- `Health`  (Max HP)
- `Magicka` (Max MP)
- `Stamina` (Max SP)
- `CarryWeight`

Current implementation uses placeholder ER-like softcaps:

- `ComputeHP(vig)`
- `ComputeMP(mnd)`
- `ComputeSP(end)`
- `ComputeCarryWeight(end)`

Relevant code:

- `skse_code/src/DerivedStats.*`

---

## 4) Published computed “AVG” ActorValues (ERCF lookup contract)

These are **computed/cache values** written as **base values** to custom ActorValues so ERCF can read them via `GetActorValue()` without duplicating formulas or calling into this plugin.

ActorValueData definitions live in:

- `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`

Publishing happens in:

- `skse_code/src/DerivedStats.cpp` in `ER::ApplyToPlayer(...)`

### 4.1 Layer 1 flat defense buckets (no armor included)

These are the “Layer 1” flat defense numbers derived from **ER level + attributes only**.

> Armor rating is intentionally excluded here because it is volatile and should be added dynamically by ERCF.

- `ER_L1DEF_PHYS_AVG`
  - Formula: `level + (STR * 0.5) + (END * 0.2)`
- `ER_L1DEF_MAGIC_AVG`
  - Formula: `level + (INT * 0.6) + (MND * 0.1)`
- `ER_L1DEF_FIRE_AVG`
  - Formula: `level + (VIG * 0.6)`
- `ER_L1DEF_LIGHTNING_AVG`
  - Formula: `level + (FTH * 0.4)`
- `ER_L1DEF_FROST_AVG`
  - Current: equals `ER_L1DEF_MAGIC_AVG` (shared bucket for now)
- `ER_L1DEF_POISON_AVG`
  - Current: equals `ER_L1DEF_MAGIC_AVG` (shared bucket for now)

### 4.2 Status threshold families (no armor included)

These are the “Layer 3” status threshold pool sizes derived from **ER level + attributes**.

- `ER_THRES_IMMUNITY_AVG`
  - Formula: `level + VIG`
- `ER_THRES_ROBUSTNESS_AVG`
  - Formula: `level + END`
- `ER_THRES_FOCUS_AVG`
  - Formula: `level + MND`
- `ER_THRES_VITALITY_AVG`
  - Formula: `level + ARC`

Madness is handled as a special pool based on max resources:

- `ER_THRES_MADNESS_AVG`
  - Formula: `(maxHP + maxMP) * 0.5`

### 4.3 Equip-load thresholds (END-driven, no gear included)

These are published so ERCF can do movement/dodge tier checks without guessing which vanilla value to read.

We use this plugin’s computed `CarryWeight` as the equip-load max proxy:

- `ER_EQUIPLOAD_MAX_AVG`
  - Formula: `carryWeight` (the value Aspects applies to vanilla `CarryWeight`)
- `ER_EQUIPLOAD_LIGHT_AVG`
  - Formula: `ER_EQUIPLOAD_MAX_AVG * LightFraction` (default `0.30`; see plugin INI `[EquipLoad]`)
- `ER_EQUIPLOAD_MEDIUM_AVG`
  - Formula: `ER_EQUIPLOAD_MAX_AVG * MediumFraction` (default `0.70`; see plugin INI `[EquipLoad]`)
- `ER_EQUIPLOAD_HEAVY_AVG`
  - Formula: `ER_EQUIPLOAD_MAX_AVG * 1.00`

### 4.3 Update timing & staleness

These `*_AVG` values are **cached**: they are correct when `ER::ApplyToPlayer(...)` runs.

Currently, this happens on:

- Confirming allocation in the Prisma level-up UI (attribute changes)
- Any other callsites that apply derived stats (if added later)

If ERCF requires “always current” values:

- Either **recompute** internally from `ER_*` AVs and `Persist` data (preferred for volatile pieces),
- Or ensure this repo publishes updates on additional triggers (load game, race switch, effects that alter max HP/MP baselines, etc.).

---

## 5) File map (where to change what)

- **Level-up gold cost curve**: `leveling_curve.md` + `skse_code/src/Economy.cpp`
- **Attributes (read/write)**: `skse_code/src/Attributes.*`
- **Derived stats computation**: `skse_code/src/DerivedStats.*`
- **Publish `*_AVG` values**: `skse_code/src/DerivedStats.cpp` (`ER::ApplyToPlayer`)
- **ActorValueData config**: `SKSE/Plugins/ActorValueData/PerkModAttributes_AVG.toml`
- **Prisma UI flow / state**: `skse_code/src/Prisma.cpp` + `skse_code/ui/src/App.tsx`

---

## 6) ERCF integration notes (recommended)

Recommended ERCF behavior:

- Treat the 8 attributes (`ER_*`) and this repo’s derived max resources as authoritative.
- Read `ER_L1DEF_*_AVG` and `ER_THRES_*_AVG` as a **baseline**.
- Add volatile contributions (armor rating, temporary buffs, perks, enchantments, race) **dynamically** inside ERCF.

