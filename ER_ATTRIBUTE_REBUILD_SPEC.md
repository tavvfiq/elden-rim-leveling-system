# ER-style attribute system (spec for this project)

This document scopes **only** the Elden Ring–style **attributes + derived stats** portion that will live in this `Aspects` project.

The separate project (`elden-rim-combat-framework`) will implement on-hit damage processing and will treat this mod as the **source of truth for the actor sheet** (max resources, defenses, thresholds, etc.).

## 0) Core principle (native SKSE loop)

The shipped implementation uses **native SKSE** (not Papyrus) for the main loop:

- **Allocate points** (PrismaUI level-up / attribute screen)
- On confirm, the plugin writes `ER_*` AVs, recomputes derived stats, and publishes sheet outputs (`ER_*_AVG`, vanilla max resources, etc.)

The old Papyrus pattern (`ApplyAttributesEvent` + `ApplyAttributeEffect.psc`) is **not** part of this repository anymore; it is only a historical analogue.

## 1) Attributes (ER set)

Target ER attributes (8):

- `ER_VIG` (Vigor)
- `ER_MND` (Mind)
- `ER_END` (Endurance)
- `ER_STR` (Strength)
- `ER_DEX` (Dexterity)
- `ER_INT` (Intelligence)
- `ER_FTH` (Faith)
- `ER_ARC` (Arcane)

Implementation detail:

- Define these as custom Actor Values in the plugin + ActorValueData config (similar to current `MM_Strength` etc.).
- Keep values as **integers** conceptually, but store as AV floats.

## 2) Progression model (decouple from Skyrim level)

We should not use `PlayerRef.GetLevel()` as the primary progression driver.

Define:

- **Rune Level (ERLevel)**: `ER_Level = 1 + ER_PointsSpent`
- **Points spent**: sum of all attribute points above base.

Storage:

- Store `ER_Level` and `ER_PointsSpent` as globals (and/or custom AVs) so other systems can read them.

Notes:

- Skyrim level can still exist, but becomes irrelevant to scaling unless explicitly opted-in.

## 3) Base values and starting class template

Elden Ring starts with class baselines.

We can support this with:

- `ER_Base_<ATTR>` globals for each attribute baseline (selected once at new game / install).
- `ER_<ATTR>` current attribute AV (baseline + allocations).

If you don’t want class selection yet:

- Use one baseline template for all characters (configurable via MCM/globals later).

## 4) Derived outputs (what this mod produces)

These are the outputs we compute in the `ApplyAttributesEvent` recalculation and publish for other mods/frameworks.

### 4.1 Vanilla actor values we set (Skyrim-visible)

- `Health` (Max HP)
- `Magicka` (Max MP)
- `Stamina` (Max SP)
- `CarryWeight` (Equip load / weight budget surrogate)

Optional (depending on how much the combat framework wants to own):

- Vanilla `DamageResist`, `MagicResist`, etc. should be treated carefully; we can instead publish custom outputs and let the combat framework apply mitigation.

### 4.2 “Sheet” outputs we publish (framework-readable)

Publish as either **globals** or **custom AVs** (recommend custom AVs if the combat framework can read them reliably):

- **Layer-1 defenses** (flat):
  - `ER_Def_Physical_L1`
  - `ER_Def_Magic_L1`
  - `ER_Def_Fire_L1`
  - `ER_Def_Lightning_L1`
  - `ER_Def_Holy_L1` (if desired)
  - `ER_Def_Frost_L1` (if your design treats frost as damage type)
  - `ER_Def_Poison_L1` (same note)
- **Threshold families** (status resistance pools):
  - `ER_Robustness`
  - `ER_Immunity`
  - `ER_Focus`
  - `ER_Madness`

Also useful:

- `ER_EquipLoad_Max` (if you want explicit rather than using CarryWeight)
- `ER_Poise` / `ER_Stance` (placeholder if you later model stagger)

## 5) ER softcaps & growth curves (to be implemented here)

This repo should own the **growth curve functions** for HP/MP/SP/etc.

We will implement “piecewise” growth (ER-style softcaps), e.g.:

- VIG softcaps: 40 / 60
- MND softcaps: 50 / 60
- END softcap: 50

Implementation approach:

- Create a helper function per derived stat:
  - `ComputeHP(vig)` using piecewise slopes
  - `ComputeMP(mnd)`
  - `ComputeSP(end)`
- Store coefficients in globals for tuning (or hardcode first, then expose to MCM later).

## 6) Two-handing bonus (STR * 1.5)

ER has: while two-handing, effective STR = `floor(STR * 1.5)`.

This project will:

- Publish both:
  - `ER_STR_Base` (actual)
  - `ER_STR_Effective` (depends on weapon state)

But: detecting two-handing state is unreliable in pure Papyrus; so:

- Default `ER_STR_Effective = ER_STR_Base`
- Allow the combat framework (SKSE) to override the effective STR when needed.

## 7) Allocation UX (menus)

Implemented path:

- **Level-up**: vanilla `LevelUpMenu` is hooked; **PrismaUI** presents 8 ER stats, pending allocation, confirm/cancel.
- **Character creation**: not covered by Papyrus in-repo anymore; add a CK/xEdit flow or a second PrismaUI entry point when you need starting templates.

Track and publish **`ERLevel`** + unspent points via **SKSE serialization** (`Persist::*`), not globals.

## 8) Interface contract with the separate damage project

The combat framework should assume:

- This project recomputes and publishes “sheet” values when the **native plugin** applies derived stats (e.g. confirm allocation). Additional triggers (load game, race change, buffs to max HP/MP) may need explicit hooks later if `*_AVG` caches must stay fresh.
- **Tuning**: plugin INI (`eldenrimlevelingsystem.ini`), not MCM Helper JSON in this repo.

Contract (read from actor):

- **Readables**:
  - attributes `ER_*`
  - derived max resources (Health/Magicka/Stamina / CarryWeight as implemented)
  - Layer-1 defense baselines (`ER_L1DEF_*_AVG`)
  - status threshold baselines (`ER_THRES_*_AVG`)

Non-goals for this project:

- On-hit interception
- Damage component splitting
- Layer-2 mitigation application
- Status buildup application/ticking

## 9) Next implementation steps (in-repo)

- Keep ActorValueData TOML in sync with any new `ER_*` / `ER_*_AVG` names.
- Wire **`[Tuning]`** INI keys into `DerivedStats` (or document they are ERCF-only) if you want full parity with old global-based sliders.
- Add **character creation** / first-load baseline if you still need class templates without Papyrus.
- Expand **refresh triggers** for `*_AVG` if staleness becomes an issue (load, shapeshift, etc.).

