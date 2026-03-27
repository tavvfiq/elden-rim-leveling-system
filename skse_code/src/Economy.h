#pragma once

namespace ER
{
	// ER-style level-up cost curve (gold acts as runes).
	std::int32_t GoldCostToLevelUp(std::int32_t fromLevel);

	// Gold is formID 0x0000000F.
	std::int32_t GetPlayerGold();
	bool TrySpendPlayerGold(std::int32_t amount);

	// Optional kill reward system (gold as runes).
	void InstallGoldKillReward();
}

