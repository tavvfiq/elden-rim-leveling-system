# Elden Ring: Rune Cost per Attribute Increase Formula

The game does not just arbitrarily pick numbers for how much a level should cost; it uses a strict, continuous mathematical equation to generate a massive exponential curve.

To calculate exactly how many Runes you need to increase an attribute by **+1**, the game engine runs your current level through a two-part formula.

---

## 1. The Level Modifier (x)
First, the game calculates a modifier variable based on your current Rune Level. The developers wanted the first 11 levels to scale very gently, so they built a threshold into this step. 

To find the modifier (x), the game uses this equation:
**x = max(0, (L - 11) * 0.02)**

* If your current level is **11 or lower**, the math inside the parenthesis would result in zero or a negative number. Because of the "max" function, the game simply hardcodes the x modifier as **0**. 
* If your current level is **12 or higher**, x begins to climb incrementally by 0.02 every single time you level up.

---

## 2. The Main Equation
Once the game has your modifier (x), it plugs it into the main calculation. The game applies a base multiplier, scales it against the square of your level (plus an offset of 81), rounds the entire decimal completely down to the nearest whole number using a "floor" function, and then adds 1.

**Rune Cost = floor((x + 0.1) * (L + 81)^2) + 1**

---

## 3. Putting It Together (Concrete Examples)
To see why the leveling cost suddenly skyrockets in the early game, let's run a Level 10 character and a Level 12 character through the exact same math.

### Example A: Buying one attribute point at Level 10
Because the player is below Level 12, the x modifier is automatically **0**.
1. Cost = floor((0 + 0.1) * (10 + 81)^2) + 1
2. Cost = floor(0.1 * (91)^2) + 1
3. Cost = floor(0.1 * 8281) + 1
4. Cost = floor(828.1) + 1
5. **Total: 829 Runes**

### Example B: Buying one attribute point at Level 12
Because the player has crossed the threshold, x is now active. (12 - 11) * 0.02 means our x value is **0.02**.
1. Cost = floor((0.02 + 0.1) * (12 + 81)^2) + 1
2. Cost = floor(0.12 * (93)^2) + 1
3. Cost = floor(0.12 * 8649) + 1
4. Cost = floor(1037.88) + 1
5. **Total: 1038 Runes**

### The Result
You can see exactly how the formula works here. At Level 12, the multiplier instantly jumps from 0.1 to 0.12, which forces the curve to aggressively angle upwards for the rest of your playthrough. By the time you reach Level 100, that x modifier has grown so large that a single attribute increase costs tens of thousands of Runes.

---

## Integration rule in this project

- There is no separate "Level Up" purchase button.
- Each attribute `+1` adds one pending level in the allocation UI.
- On **Confirm**, the plugin:
  1. Sums rune/gold costs for each pending level step (`L`, `L+1`, `L+2`, ...).
  2. Charges the total once.
  3. Applies all pending attribute increases.
  4. Increments ER Level by the number of confirmed pending points.