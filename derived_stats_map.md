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

## 2. Computed Baselines

While your overall Rune Level provides a tiny global increase to all defenses, specific attributes provide targeted elemental or physical defense. 

Here are the exact numerical gains per point for each defensive attribute, showing exactly where the "soft caps" and scaling spikes occur.

---

### Strength ➔ Physical Defense
Strength provides the most consistent scaling, with a massive defensive spike right in the mid-to-late game bracket.

* **Stats 1 to 30:** You gain ~0.33 Defense per attribute point.
* **Stats 31 to 40:** You gain 0.50 Defense per attribute point.
* **Stats 41 to 60:** You gain 0.75 Defense per attribute point. *(This is where leveling Strength gives you the absolute best defensive return).*
* **Stats 61 to 99:** You gain ~0.25 Defense per attribute point.

---

### Vigor ➔ Fire Defense
Vigor is unique. The developers designed it so that your Fire Defense heavily spikes precisely when your HP gains also spike, making you incredibly tanky against fire in the early-mid game.

* **Stats 1 to 30:** You gain ~0.66 Defense per attribute point.
* **Stats 31 to 40:** You gain 2.00 Defense per attribute point. *(A massive defensive spike!)*
* **Stats 41 to 60:** You gain 1.00 Defense per attribute point.
* **Stats 61 to 99:** You gain 0.25 Defense per attribute point.

---

### Arcane ➔ Holy Defense 
Unlike Strength or Vigor, Arcane is extremely front-loaded. You get the vast majority of your Holy Defense in the very early levels, and the returns diminish the higher you go.

* **Stats 1 to 20:** You gain 2.00 Defense per attribute point. *(Huge early returns).*
* **Stats 21 to 35:** You gain ~0.66 Defense per attribute point.
* **Stats 36 to 60:** You gain 0.40 Defense per attribute point.
* **Stats 61 to 99:** You gain 0.25 Defense per attribute point.

---

### Intelligence ➔ Magic Defense
Intelligence follows an almost identical curve to Arcane. It is heavily front-loaded to protect low-level magic users from getting instantly killed by enemy sorcerers.

* **Stats 1 to 20:** You gain 2.00 Defense per attribute point. 
* **Stats 21 to 35:** You gain ~0.66 Defense per attribute point.
* **Stats 36 to 60:** You gain 0.40 Defense per attribute point.
* **Stats 61 to 99:** You gain 0.25 Defense per attribute point.

---

### The Global "Rune Level" Defense Curve
Even if you are leveling a stat that gives *no* specific elemental defense (like Dexterity or Faith), you still gain a tiny bit of universal flat defense simply because your overall character level went up. This curve applies to all damage types (Physical, Magic, Fire, Lightning, Holy).

* **Levels 1 to 71:** You gain 0.40 Defense per level.
* **Levels 72 to 91:** You gain 1.00 Defense per level.
* **Levels 92 to 161:** You gain ~0.21 Defense per level.
* **Levels 162 to 713:** You gain ~0.03 Defense per level. *(Effectively zero).*

## The Universal Resistance Curve
*(Applies to Vigor, Endurance, Mind, and Arcane)*

* **Stats 1 to 30:** You gain ~1.5 Resistance per attribute point.
* **Stats 31 to 40:** You gain ~3.0 Resistance per attribute point. *(The massive spike! You get double the value per point in this bracket).*
* **Stats 41 to 60:** You gain ~1.0 Resistance per attribute point. *(Heavy diminishing returns begin here).*
* **Stats 61 to 99:** You gain ~0.25 Resistance per attribute point. *(Effectively dead levels for resistance).*

### Vigor ➔ Immunity
* **Protects Against:** Poison and Scarlet Rot.
* **The Result:** Because almost every player is heavily incentivized to level Vigor to 40+ just to survive taking hits, you will naturally develop incredibly high Immunity without even trying. This makes late-game rot swamps noticeably more forgiving than early-game ones.

### Endurance ➔ Robustness
* **Protects Against:** Hemorrhage (Bleed) and Frostbite.
* **The Result:** Robustness is arguably the most important resistance in the game (especially in PvP) due to the absolute dominance of Bleed builds. Hitting that level 40 Endurance soft cap gives your invisible pool a massive buffer against rapid katana combos.

### Mind ➔ Focus
* **Protects Against:** Sleep and Madness.
* **The Result:** Focus is highly situational, but having at least 30-40 Mind makes navigating madness-heavy areas (like the Frenzied Flame Village) or fighting certain madness-inflicting invaders significantly less stressful.

---

## 3. Dynamic Modifiers (The "ERCF" Contract)

Because `Aspects` intentionally excludes armor and buffs from the published `_AVG` values to prevent staleness, `ERCF` must calculate the final combat math on the fly:

* **Final Layer 1 Defense:** `ER_L1DEF_*_AVG` (from Aspects) + `Armor Rating Contributions` + `Active Spell Buffs`.
* **Final Status Threshold:** `ER_THRES_*_AVG` (from Aspects) + `Jewelry/Armor Enchantments`.
* **Damage Output:** `Weapon Base Damage` * `(Scaling Factor based on ER_STR / ER_DEX / ER_INT / ER_FTH)`.