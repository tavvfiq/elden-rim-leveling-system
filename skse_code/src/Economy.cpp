#include "Economy.h"

#include "pch.h"

#include <cmath>
#include <limits>

namespace ER
{
	std::int32_t GoldCostToLevelUp(std::int32_t fromLevel)
	{
		// Elden Ring rune cost to go from current level L to L+1.
		// Spec: leveling_curve.md in this repo.
		//
		//   x = max(0, (L - 11) * 0.02)
		//   cost = floor((x + 0.1) * (L + 81)^2) + 1
		//
		// L is the player's current level before spending (same as Persist::GetERLevel()).
		fromLevel = std::max(1, fromLevel);
		const double L = static_cast<double>(fromLevel);
		const double x = std::max(0.0, (L - 11.0) * 0.02);
		const double sum = L + 81.0;
		const double inner = (x + 0.1) * sum * sum;
		const auto floored = static_cast<std::int64_t>(std::floor(inner));
		const auto cost = floored + 1;
		return static_cast<std::int32_t>(std::min<std::int64_t>(cost, static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max())));
	}

	std::int32_t GetPlayerGold()
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return 0;
		}

		auto* gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000F);
		if (!gold) {
			return 0;
		}

		return static_cast<std::int32_t>(player->GetItemCount(gold));
	}

	bool TrySpendPlayerGold(std::int32_t amount)
	{
		if (amount <= 0) {
			return true;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return false;
		}

		auto* gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000F);
		if (!gold) {
			return false;
		}

		const auto current = static_cast<std::int32_t>(player->GetItemCount(gold));
		if (current < amount) {
			return false;
		}

		player->RemoveItem(gold, amount, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
		return true;
	}
}

