# Gold Drop System

This document outlines the design and mathematical model for the "Rune" economy, where enemies drop Gold directly upon death, scaling appropriately from early-game to late-game.

## 1) The Core Formula

To ensure smooth scaling that accounts for both an enemy's level and their inherent "tankiness," the formula calculates a base value from Level and Max HP, then applies tier-based multipliers.

`BaseGold = (EnemyLevel * BaseMultiplier) + (EnemyMaxHP * HealthWeight)`
`FinalGold = BaseGold * TierMultiplier * ContextMultiplier`

## 2) Variable Definitions

### 2.1 Enemy Level (`EnemyLevel`)
Represents the raw scaling of the enemy. 
- **BaseMultiplier:** `5.0` (Recommended)

### 2.2 Maximum Health (`EnemyMaxHP`)
Accounts for the difference between fragile enemies (e.g., mages) and tanky enemies (e.g., bears) of the exact same level.
- **HealthWeight:** `0.15` (Recommended)

### 2.3 Tier Multiplier (`TierMultiplier`)
Provides massive payouts for elite enemies and bosses based on vanilla Skyrim keywords or specific flags.
- **Standard (`1.0x`):** Default for most standard NPCs and creatures (Bandits, Draugr, Wolves).
- **Elite/Giant (`2.5x`):** Applied to large or difficult creatures. Checked via keywords like `ActorTypeGiant` or high-level leveled list flags.
- **Boss (`10.0x`):** Applied to major encounters. Checked via keywords like `ActorTypeDragon` or `LocTypeBoss`.

### 2.4 Context Multiplier (`ContextMultiplier`)
Optional situational bonuses to reward skillful play.
- **Overkill Bonus (`1.2x`):** Applied if the killing blow deals damage exceeding 150% of the target's Max HP.
- **Standard Hit (`1.0x`):** Default state.

## 3) Worked Examples

### Example A: Low-level Fodder (Level 5 Bandit)
- **Stats:** Level 5, 100 HP
- **Tier:** Standard (1.0x)
- **Calculation:** `(5 * 5) + (100 * 0.15) = 25 + 15`
- **Final Drop:** `40 Gold`

### Example B: Mid-level Threat (Level 25 Draugr Scourge)
- **Stats:** Level 25, 500 HP
- **Tier:** Standard (1.0x)
- **Calculation:** `(25 * 5) + (500 * 0.15) = 125 + 75`
- **Final Drop:** `200 Gold`

### Example C: Elite Creature (Level 40 Giant)
- **Stats:** Level 40, 1200 HP
- **Tier:** Elite (2.5x)
- **Calculation:** `[(40 * 5) + (1200 * 0.15)] * 2.5 = [200 + 180] * 2.5`
- **Final Drop:** `950 Gold`

### Example D: Major Boss (Level 50 Ancient Dragon)
- **Stats:** Level 50, 3000 HP
- **Tier:** Boss (10.0x)
- **Calculation:** `[(50 * 5) + (3000 * 0.15)] * 10.0 = [250 + 450] * 10.0`
- **Final Drop:** `7000 Gold`

## 4) ERCF Implementation Notes

1. **Event Hook:** Hook into the `OnDeath` event within the `ERCF` plugin.
2. **Retrieve Data:** Fetch `GetLevel()` and `GetBaseActorValue("Health")` from the dying actor.
3. **Keyword Check:** Use `HasKeyword()` to determine the correct `TierMultiplier`.
4. **Reward Player:** Bypass vanilla looting by automatically depositing the calculated `FinalGold` directly into the player's inventory (`Game.GetPlayer().AddItem(Gold001, FinalGold)`).
5. **Feedback:** Play a custom golden particle effect and sound effect at the player's location to simulate absorbing runes.