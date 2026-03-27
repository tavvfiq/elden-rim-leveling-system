#pragma once

#include "Attributes.h"

namespace ER
{
	struct DerivedStats
	{
		std::int32_t maxHP{ 100 };
		std::int32_t maxMP{ 100 };
		std::int32_t maxSP{ 100 };
		std::int32_t carryWeight{ 300 };
	};

	// Layer-1 defenses + status thresholds + equip-load tiers (derived_stats_map.md).
	// Single source for AV publish + Prisma JSON preview.
	struct PublishedSheetAVGs
	{
		float l1Phys{};
		float l1Magic{};
		float l1Fire{};
		float l1Lightning{};
		float l1Frost{};
		float l1Poison{};
		float thImmunity{};
		float thRobustness{};
		float thFocus{};
		float thVitality{};
		float thMadness{};
		float equipLoadMax{};
		float equipLoadLight{};
		float equipLoadMedium{};
		float equipLoadHeavy{};
	};

	struct StatsSnapshot
	{
		AttributeSet attrs{};
		std::int32_t erLevel{ 1 };
		DerivedStats derived{};
		PublishedSheetAVGs sheet{};
	};

	PublishedSheetAVGs ComputePublishedSheetAVGs(const AttributeSet& attrs, std::int32_t erLevel, const DerivedStats& derived);
	StatsSnapshot BuildStatsSnapshot(const AttributeSet& attrs, std::int32_t erLevel);
	StatsSnapshot GetCurrentStatsSnapshot();

	// Initial, placeholder “ER-like” softcap curves.
	// We can replace coefficients later to match ER precisely.
	std::int32_t ComputeHP(std::int32_t vig);
	std::int32_t ComputeMP(std::int32_t mnd);
	std::int32_t ComputeSP(std::int32_t end);
	std::int32_t ComputeCarryWeight(std::int32_t end);

	DerivedStats ComputeDerived(const AttributeSet& attrs);
	void ApplyToPlayer(const DerivedStats& stats);
	void ApplyToPlayer(const DerivedStats& stats, const AttributeSet& attrs, std::int32_t erLevel);
}

