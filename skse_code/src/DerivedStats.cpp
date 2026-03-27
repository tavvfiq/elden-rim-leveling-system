#include "DerivedStats.h"

#include "pch.h"

#include "Config.h"
#include "Serialization.h"

namespace
{
	// Simple piecewise linear curve helper.
	std::int32_t Piecewise(
		std::int32_t x,
		std::initializer_list<std::pair<std::int32_t, float>> segments,
		std::int32_t baseValue)
	{
		// segments: (capInclusive, slopePerPoint)
		std::int32_t value = baseValue;
		std::int32_t prevCap = 0;

		for (auto [cap, slope] : segments) {
			const std::int32_t start = prevCap + 1;
			const std::int32_t end = cap;
			if (x < start) {
				break;
			}

			const std::int32_t used = std::min(x, end) - start + 1;
			value += static_cast<std::int32_t>(std::lround(static_cast<float>(used) * slope));
			prevCap = cap;

			if (x <= cap) {
				break;
			}
		}

		return value;
	}

	float PiecewiseFloat(
		std::int32_t x,
		std::initializer_list<std::pair<std::int32_t, float>> segments)
	{
		x = std::max(0, x);
		float value = 0.0f;
		std::int32_t prevCap = 0;
		for (auto [cap, slope] : segments) {
			const std::int32_t start = prevCap + 1;
			const std::int32_t end = cap;
			if (x < start) {
				break;
			}
			const std::int32_t used = std::min(x, end) - start + 1;
			value += static_cast<float>(used) * slope;
			prevCap = cap;
			if (x <= cap) {
				break;
			}
		}
		return value;
	}

	float ComputeLayer1LevelIncrement(std::int32_t level)
	{
		// Global defensive growth from Rune Level.
		const std::int32_t l = std::max(1, level);
		const std::int32_t seg1 = std::max(0, std::min(l, 71) - 1);    // levels 2..71
		const std::int32_t seg2 = std::max(0, std::min(l, 91) - 71);   // levels 72..91
		const std::int32_t seg3 = std::max(0, std::min(l, 161) - 91);  // levels 92..161
		const std::int32_t seg4 = std::max(0, std::min(l, 713) - 161); // levels 162..713
		return static_cast<float>(seg1) * 0.40f +
		       static_cast<float>(seg2) * 1.00f +
		       static_cast<float>(seg3) * 0.21f +
		       static_cast<float>(seg4) * 0.03f;
	}

	float ComputeLayer1BaseDefense(std::int32_t level)
	{
		level = std::max(1, level);
		return 10.0f + (static_cast<float>(level) + 78.0f) / 2.483f;
	}

	float ComputePhysicalDefenseFromSTR(std::int32_t str)
	{
		return PiecewiseFloat(str, {
			{ 30, 0.33f },
			{ 40, 0.50f },
			{ 60, 0.75f },
			{ 99, 0.25f },
		});
	}

	float ComputeFireDefenseFromVIG(std::int32_t vig)
	{
		return PiecewiseFloat(vig, {
			{ 30, 0.66f },
			{ 40, 2.00f },
			{ 60, 1.00f },
			{ 99, 0.25f },
		});
	}

	float ComputeMagicDefenseFromINT(std::int32_t intl)
	{
		return PiecewiseFloat(intl, {
			{ 20, 2.00f },
			{ 35, 0.66f },
			{ 60, 0.40f },
			{ 99, 0.25f },
		});
	}

	float ComputeHolyDefenseFromARC(std::int32_t arc)
	{
		return PiecewiseFloat(arc, {
			{ 20, 2.00f },
			{ 35, 0.66f },
			{ 60, 0.40f },
			{ 99, 0.25f },
		});
	}

	float ComputeUniversalResistance(std::int32_t stat)
	{
		return PiecewiseFloat(stat, {
			{ 30, 1.50f },
			{ 40, 3.00f },
			{ 60, 1.00f },
			{ 99, 0.25f },
		});
	}
}

namespace ER
{
	std::int32_t ComputeHP(std::int32_t vig)
	{
		// Placeholder: faster growth early, softer after 40/60.
		// Base assumes “1” is minimal; treat <=0 as 0 contribution.
		vig = std::max(0, vig);
		return Piecewise(vig,
			{
				{ 40, 25.0f },
				{ 60, 12.0f },
				{ 99, 6.0f },
			},
			100);
	}

	std::int32_t ComputeMP(std::int32_t mnd)
	{
		mnd = std::max(0, mnd);
		return Piecewise(mnd,
			{
				{ 50, 10.0f },
				{ 60, 6.0f },
				{ 99, 3.0f },
			},
			100);
	}

	std::int32_t ComputeSP(std::int32_t end)
	{
		end = std::max(0, end);
		return Piecewise(end,
			{
				{ 50, 6.0f },
				{ 99, 2.0f },
			},
			100);
	}

	std::int32_t ComputeCarryWeight(std::int32_t end)
	{
		end = std::max(0, end);
		// Placeholder mapping for now.
		return 300 + static_cast<std::int32_t>(std::lround(static_cast<float>(end) * 5.0f));
	}

	DerivedStats ComputeDerived(const AttributeSet& attrs)
	{
		DerivedStats d;
		d.maxHP = ComputeHP(attrs.vig);
		d.maxMP = ComputeMP(attrs.mnd);
		d.maxSP = ComputeSP(attrs.end);
		d.carryWeight = ComputeCarryWeight(attrs.end);
		return d;
	}

	PublishedSheetAVGs ComputePublishedSheetAVGs(const AttributeSet& attrs, std::int32_t erLevel, const DerivedStats& derived)
	{
		const std::int32_t level = std::max(1, erLevel);
		PublishedSheetAVGs s;
		const float baseDef = ComputeLayer1BaseDefense(level);
		const float globalDef = ComputeLayer1LevelIncrement(level);

		s.l1Phys = baseDef + globalDef + ComputePhysicalDefenseFromSTR(attrs.str);
		s.l1Magic = baseDef + globalDef + ComputeMagicDefenseFromINT(attrs.intl);
		s.l1Fire = baseDef + globalDef + ComputeFireDefenseFromVIG(attrs.vig);
		s.l1Lightning = baseDef + globalDef + ComputeHolyDefenseFromARC(attrs.arc);
		s.l1Frost = s.l1Magic;
		s.l1Poison = s.l1Magic;

		s.thImmunity = ComputeUniversalResistance(attrs.vig);
		s.thRobustness = ComputeUniversalResistance(attrs.end);
		s.thFocus = ComputeUniversalResistance(attrs.mnd);
		s.thVitality = ComputeUniversalResistance(attrs.arc);
		s.thMadness = static_cast<float>(derived.maxHP + derived.maxMP) * 0.5f;

		s.equipLoadMax = static_cast<float>(derived.carryWeight);
		s.equipLoadLight = s.equipLoadMax * ER::Config::EquipLoadLightFraction();
		s.equipLoadMedium = s.equipLoadMax * ER::Config::EquipLoadMediumFraction();
		s.equipLoadHeavy = s.equipLoadMax * 1.00f;

		return s;
	}

	StatsSnapshot BuildStatsSnapshot(const AttributeSet& attrs, std::int32_t erLevel)
	{
		StatsSnapshot snapshot;
		snapshot.attrs = attrs;
		snapshot.erLevel = std::max(1, erLevel);
		snapshot.derived = ComputeDerived(attrs);
		snapshot.sheet = ComputePublishedSheetAVGs(attrs, snapshot.erLevel, snapshot.derived);
		return snapshot;
	}

	StatsSnapshot GetCurrentStatsSnapshot()
	{
		return BuildStatsSnapshot(Persist::GetAttributes(), Persist::GetERLevel());
	}

	void ApplyToPlayer(const DerivedStats& stats)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kHealth, static_cast<float>(stats.maxHP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kMagicka, static_cast<float>(stats.maxMP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kStamina, static_cast<float>(stats.maxSP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kCarryWeight, static_cast<float>(stats.carryWeight));

		const auto attrs = ER::GetAll(player);
		const std::int32_t level = Persist::GetERLevel();
		ApplyToPlayer(stats, attrs, level);
	}

	void ApplyToPlayer(const DerivedStats& stats, const AttributeSet& attrs, std::int32_t erLevel)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kHealth, static_cast<float>(stats.maxHP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kMagicka, static_cast<float>(stats.maxMP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kStamina, static_cast<float>(stats.maxSP));
		player->AsActorValueOwner()->SetBaseActorValue(RE::ActorValue::kCarryWeight, static_cast<float>(stats.carryWeight));
		(void)attrs;
		(void)erLevel;
	}
}

