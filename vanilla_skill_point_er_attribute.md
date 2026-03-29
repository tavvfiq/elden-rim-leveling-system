# NPC Skill-to-Attribute Conversion Mapping

## Overview
This document defines how vanilla Skyrim skills are converted into the 8 custom `ER_` ActorValues for humanoid NPCs. This ensures that enemies scale dynamically with the new Elden Ring combat mathematics (damage, defense, and status thresholds) based on their vanilla classes.

## Eligibility (humanoid gate)

- An actor is eligible for **skill-derived** ER attributes only if it has the vanilla keyword **`ActorTypeNPC`**.
- The **player** is **never** treated as an NPC for this path: the player’s eight stats always come from **SKSE persistence** (`Persist`), not from this mapping.
- Actors without `ActorTypeNPC` (e.g. most creatures) are not covered by this document unless you add a separate rule set.

Code: `ER::IsNPCAttributeEligible(actor)` in `skse_code/src/Attributes.*` encodes the above (player excluded + `ActorTypeNPC`).

## Implementation (plugin)

- Skill → attribute mapping: `ComputeNPCAttributesFromVanillaSkills` in `skse_code/src/Attributes.cpp` (uses `GetBaseActorValue` on vanilla skill `ActorValue`s, clamps 0–99).
- Unified read path: `ER::GetAll(actor)` — player → `Persist`, eligible NPC → mapping, else → zeros.
- Effective level for sheet math: `ER::GetActorEffectiveLevel(actor)` — player → persisted ER level, else → `actor->GetLevel()` (min 1).
- Full computed sheet for any actor: `ER::GetStatsSnapshotForActor(actor)` in `skse_code/src/DerivedStats.cpp`.
- Native API: `IERAS1::getStatsSnapshotForActor(actorPtr, out)` in `skse_code/src/ERAS_API.h` (`actorPtr` = `RE::Actor*`).
- Papyrus: `ERAS.GetActorERAttr`, `GetActorERLevelForActor`, `IsActorERDerivedFromVanilla` in `SKSE/Plugins/Source/ERLS.psc`.

## 1) The Skill Mapping Groups

Each of the 8 Elden Ring attributes is derived from an average of 2 to 3 logically grouped vanilla Skyrim skills. 

### 1.1 The Combat Attributes (Offense & Scaling)
These attributes define the NPC's weapon damage and specific physical defenses.

* **Strength (`ER_STR`)** -> *The Heavy Bruiser*
    * **Contributing Skills:** `Two-Handed`, `Heavy Armor`, `Smithing`
    * *Logic:* NPCs with high Strength wield colossal weapons and wear heavy plates. 
* **Dexterity (`ER_DEX`)** -> *The Agile Fighter*
    * **Contributing Skills:** `One-Handed`, `Archery`, `Sneak`
    * *Logic:* Governs fast strikes, ranged combat, and precision.
* **Intelligence (`ER_INT`)** -> *The Sorcerer*
    * **Contributing Skills:** `Destruction`, `Alteration`
    * *Logic:* Governs raw elemental damage and magical shielding.
* **Faith (`ER_FTH`)** -> *The Cleric / Cultist*
    * **Contributing Skills:** `Restoration`, `Conjuration`
    * *Logic:* Governs healing, holy magic, and summoning arts.
* **Arcane (`ER_ARC`)** -> *The Occultist / Assassin*
    * **Contributing Skills:** `Alchemy`, `Pickpocket`, `Illusion`
    * *Logic:* Governs poisons, status effects, and mind manipulation. 

### 1.2 The Base Attributes (Survival & Resources)
These attributes dictate the NPC's derived stats (HP, MP, SP) and status resistances. Because Skyrim doesn't have direct skills for "Health" or "Stamina", we derive these from defensive and conditioning skills, plus a baseline level bonus.

* **Vigor (`ER_VIG`)** -> *Health & Fire Defense*
    * **Contributing Skills:** `Block`
    * *Logic:* Block represents physical resilience. Vigor is heavily supplemented by the NPC's Level to ensure health scales properly.
* **Endurance (`ER_END`)** -> *Stamina & Equip Load*
    * **Contributing Skills:** `Light Armor`
    * *Logic:* Light armor requires physical conditioning and stamina management.
* **Mind (`ER_MND`)** -> *Magicka & Focus*
    * **Contributing Skills:** `Enchanting`, `Speech`
    * *Logic:* Represents mental fortitude, focus, and willpower.

---

## 2) The Conversion Formulas (Balancing)

To keep the attributes within the standard Elden Ring scale (10 to 99), use a simple averaging formula for the skills. 

For the formula, let `S_1`, `S_2`, etc., represent the vanilla skill levels.

### Offensive Attributes
* `ER_STR = (TwoHanded + HeavyArmor + Smithing) / 3`
* `ER_DEX = (OneHanded + Archery + Sneak) / 3`
* `ER_INT = (Destruction + Alteration) / 2`
* `ER_FTH = (Restoration + Conjuration) / 2`
* `ER_ARC = (Alchemy + Pickpocket + Illusion) / 3`

### Resource Attributes (Level-Weighted)
Because an NPC's health and stamina should always scale with their level (so high-level mages don't have 10 Vigor), we weight their level into the resource attributes.
* `ER_VIG = 10 + (Level * 0.5) + (Block * 0.5)`
* `ER_END = 10 + (Level * 0.5) + (LightArmor * 0.5)`
* `ER_MND = 10 + (Level * 0.5) + ((Enchanting + Speech) / 4)`

*(Note: Apply a hard clamp/max to all results at `99` to respect the Elden Ring attribute ceiling).*

---

## 3) Worked Examples (Proving the Math)

Let's run Skyrim's vanilla NPC classes through the formula to see their Elden Ring build archetypes.

### Example A: Level 20 Bandit Melee (Class: CombatWarrior1H)
* *Vanilla Stats:* One-Handed (60), Light Armor (55), Block (40), Sneak (30), Heavy Armor (20).
* **ER_DEX:** `(60 + 15 + 30) / 3` = **35 DEX** (Primary damage stat)
* **ER_END:** `10 + (20 * 0.5) + (55 * 0.5)` = **47 END** (High stamina/robustness)
* **ER_STR:** `(15 + 20 + 15) / 3` = **16 STR** (Low strength)
* **ER_VIG:** `10 + (20 * 0.5) + (40 * 0.5)` = **40 VIG** (Solid health pool)
* *Result:* A Dexterity/Endurance build. Perfectly matches a lightly armored Bandit.

### Example B: Level 40 Master Necromancer (Class: CombatMageConjuration)
* *Vanilla Stats:* Conjuration (85), Alteration (70), Destruction (50), Restoration (40), Light Armor (15).
* **ER_FTH:** `(40 + 85) / 2` = **62 FTH** (Primary damage stat - High Incantation scaling)
* **ER_INT:** `(50 + 70) / 2` = **60 INT** (Secondary damage stat)
* **ER_MND:** `10 + (40 * 0.5) + (30 / 4)` = **37 MND** (High Magicka pool)
* **ER_VIG:** `10 + (40 * 0.5) + (15 * 0.5)` = **37 VIG** (Squishy relative to level)
* *Result:* An Int/Faith hybrid caster (similar to an Elden Ring Death Prince build).

### Example C: Level 50 Orc Bandit Chief (Class: CombatWarrior2H)
* *Vanilla Stats:* Two-Handed (90), Heavy Armor (85), Block (70), Smithing (50).
* **ER_STR:** `(90 + 85 + 50) / 3` = **75 STR** (Massive heavy damage)
* **ER_VIG:** `10 + (50 * 0.5) + (70 * 0.5)` = **70 VIG** (Massive health and fire resist)
* **ER_DEX:** `(15 + 15 + 15) / 3` = **15 DEX** (No finesse)
* *Result:* A pure Strength/Vigor tank build.