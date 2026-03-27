# Gold Kill Reward System

Implemented in `skse_code/src/Economy.cpp`.

## Enable toggle

`SKSE/Plugins/eldenrimlevelingsystem.ini`

```ini
EnableGoldKillDrops = true
```

## Formula

```text
BaseGold  = EnemyLevel * baseMultiplier + EnemyMaxHP * healthWeight
FinalGold = BaseGold * tierMultiplier * contextMultiplier
```

Values are floored/clamped to integer payout in code.

## External tuning file

`SKSE/Plugins/eldenrimlevelingsystem_gold_kill.json`

Supported fields:

- `baseMultiplier`
- `healthWeight`
- `defaultTierMultiplier`
- `contextMultiplier`
- `keywordMultipliers` (object map of `keyword -> multiplier`)

Example:

```json
{
  "baseMultiplier": 5.0,
  "healthWeight": 0.15,
  "defaultTierMultiplier": 1.0,
  "contextMultiplier": 1.0,
  "keywordMultipliers": {
    "ActorTypeDragon": 10.0,
    "ActorTypeGiant": 2.5
  }
}
```

## Runtime behavior

- Hook listens to death events.
- Gold is awarded directly to player inventory on valid player-caused kills.
- Tier multiplier is selected from keyword map; falls back to `defaultTierMultiplier`.