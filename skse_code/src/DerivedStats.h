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

	// Initial, placeholder “ER-like” softcap curves.
	// We can replace coefficients later to match ER precisely.
	std::int32_t ComputeHP(std::int32_t vig);
	std::int32_t ComputeMP(std::int32_t mnd);
	std::int32_t ComputeSP(std::int32_t end);
	std::int32_t ComputeCarryWeight(std::int32_t end);

	DerivedStats ComputeDerived(const AttributeSet& attrs);
	void ApplyToPlayer(const DerivedStats& stats);
}

