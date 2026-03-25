# Aspects & ERCF Attribute Mapping

This document outlines how the 8 custom `ER_*` ActorValues govern the player's derived stats, defenses, and offensive scaling, entirely replacing vanilla Skyrim skills and perks.

## 1. Vigor (`ER_VIG`)
* **Derived Stat:** Directly computes vanilla `Health` (Max HP).
* **Layer 1 Defense:** Primary contributor to `ER_L1DEF_FIRE_AVG`.
* **Status Threshold:** Primary contributor to `ER_THRES_IMMUNITY_AVG` (Poison/Rot).

## 2. Mind (`ER_MND`)
* **Derived Stat:** Directly computes vanilla `Magicka` (Max MP).
* **Layer 1 Defense:** Minor contributor to `ER_L1DEF_MAGIC_AVG`.
* **Status Threshold:** Primary contributor to `ER_THRES_FOCUS_AVG` (Sleep).

## 3. Endurance (`ER_END`)
* **Derived Stat:** Directly computes vanilla `Stamina` (Max SP) and `CarryWeight`.
* **Layer 1 Defense:** Minor contributor to `ER_L1DEF_PHYS_AVG`.
* **Status Threshold:** Primary contributor to `ER_THRES_ROBUSTNESS_AVG` (Bleed/Frost).
* **Dynamic (ERCF):** Determines Equip Load thresholds for dodging/movement speed.

## 4. Strength (`ER_STR`)
* **Layer 1 Defense:** Primary contributor to `ER_L1DEF_PHYS_AVG`.
* **Offensive Scaling (ERCF):** Increases Physical damage for **Strike** and **Standard** weapons. 
* **Dynamic (ERCF):** Multiplies effective `ER_STR` by 1.5x when a weapon is two-handed, affecting both damage and L1 Physical Defense dynamically during combat.

## 5. Dexterity (`ER_DEX`)
* **Offensive Scaling (ERCF):** Increases Physical damage for **Slash** and **Pierce** weapons (Swords, Daggers, Bows).
* **Dynamic (ERCF):** Reduces the casting time/animation speed of spells and reduces physical fall damage. 

## 6. Intelligence (`ER_INT`)
* **Layer 1 Defense:** Primary contributor to `ER_L1DEF_MAGIC_AVG`. Note: Currently also serves as the bucket for Frost and Poison defense.
* **Offensive Scaling (ERCF):** Increases the damage and magnitude of Sorceries (Magic/Frost damage) and Magic-infused weapons.

## 7. Faith (`ER_FTH`)
* **Layer 1 Defense:** Primary contributor to `ER_L1DEF_LIGHTNING_AVG`.
* **Offensive Scaling (ERCF):** Increases the damage of Incantations (Fire/Lightning/Sun damage) and Sacred/Fire-infused weapons.
* **Dynamic (ERCF):** Scales the potency of healing spells and duration of buffs.

## 8. Arcane (`ER_ARC`)
* **Status Threshold:** Primary contributor to `ER_THRES_VITALITY_AVG`.
* **Offensive Scaling (ERCF):** Increases the application rate (status buildup) of Poison, Bleed, and Rot when using weapons infused with Arcane scaling.
* **Dynamic (ERCF):** Replaces vanilla looting odds to increase item discovery rates.

---

## 2. Computed Baselines (The "Aspects" Contract)

As defined in the `Aspects` system, these values are published as base metrics. ERCF will read these and add dynamic modifiers.

### Defense Baselines (Layer 1)
* **Physical:** `ERLevel + (ER_STR * 0.5) + (ER_END * 0.2)`
* **Magic/Frost/Poison:** `ERLevel + (ER_INT * 0.6) + (ER_MND * 0.1)`
* **Fire:** `ERLevel + (ER_VIG * 0.6)`
* **Lightning:** `ERLevel + (ER_FTH * 0.4)`

### Status Thresholds (Layer 3)
* **Immunity:** `ERLevel + ER_VIG`
* **Robustness:** `ERLevel + ER_END`
* **Focus:** `ERLevel + ER_MND`
* **Vitality:** `ERLevel + ER_ARC`
* **Madness:** `(MaxHP + MaxMP) * 0.5`

---

## 3. Dynamic Modifiers (The "ERCF" Contract)

Because `Aspects` intentionally excludes armor and buffs from the published `_AVG` values to prevent staleness, `ERCF` must calculate the final combat math on the fly:

* **Final Layer 1 Defense:** `ER_L1DEF_*_AVG` (from Aspects) + `Armor Rating Contributions` + `Active Spell Buffs`.
* **Final Status Threshold:** `ER_THRES_*_AVG` (from Aspects) + `Jewelry/Armor Enchantments`.
* **Damage Output:** `Weapon Base Damage` * `(Scaling Factor based on ER_STR / ER_DEX / ER_INT / ER_FTH)`.