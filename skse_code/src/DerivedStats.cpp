#include "DerivedStats.h"

#include "pch.h"

#include "Config.h"
#include "Serialization.h"

namespace
{
	RE::ActorValue ResolveActorValueByName(std::string_view name)
	{
		if (name.empty()) {
			return RE::ActorValue::kNone;
		}

		auto* list = RE::ActorValueList::GetSingleton();
		if (!list) {
			return RE::ActorValue::kNone;
		}

		return list->LookupActorValueByName(name);
	}

	void SetBaseByName(RE::Actor* actor, std::string_view name, float value)
	{
		if (!actor) {
			return;
		}

		const auto av = ResolveActorValueByName(name);
		if (av == RE::ActorValue::kNone) {
			return;
		}

		actor->AsActorValueOwner()->SetBaseActorValue(av, value);
	}

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

		s.l1Phys =
			static_cast<float>(level) +
			static_cast<float>(attrs.str) * 0.5f +
			static_cast<float>(attrs.end) * 0.2f;

		s.l1Magic =
			static_cast<float>(level) +
			static_cast<float>(attrs.intl) * 0.6f +
			static_cast<float>(attrs.mnd) * 0.1f;

		s.l1Fire = static_cast<float>(level) + static_cast<float>(attrs.vig) * 0.6f;

		s.l1Lightning = static_cast<float>(level) + static_cast<float>(attrs.fth) * 0.4f;

		s.l1Frost = s.l1Magic;
		s.l1Poison = s.l1Magic;

		s.thImmunity = static_cast<float>(level + attrs.vig);
		s.thRobustness = static_cast<float>(level + attrs.end);
		s.thFocus = static_cast<float>(level + attrs.mnd);
		s.thVitality = static_cast<float>(level + attrs.arc);
		s.thMadness = static_cast<float>(derived.maxHP + derived.maxMP) * 0.5f;

		s.equipLoadMax = static_cast<float>(derived.carryWeight);
		s.equipLoadLight = s.equipLoadMax * ER::Config::EquipLoadLightFraction();
		s.equipLoadMedium = s.equipLoadMax * ER::Config::EquipLoadMediumFraction();
		s.equipLoadHeavy = s.equipLoadMax * 1.00f;

		return s;
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
		const auto sheet = ComputePublishedSheetAVGs(attrs, level, stats);

		SetBaseByName(player, "ER_L1DEF_PHYS_AVG", sheet.l1Phys);
		SetBaseByName(player, "ER_L1DEF_MAGIC_AVG", sheet.l1Magic);
		SetBaseByName(player, "ER_L1DEF_FIRE_AVG", sheet.l1Fire);
		SetBaseByName(player, "ER_L1DEF_FROST_AVG", sheet.l1Frost);
		SetBaseByName(player, "ER_L1DEF_POISON_AVG", sheet.l1Poison);
		SetBaseByName(player, "ER_L1DEF_LIGHTNING_AVG", sheet.l1Lightning);

		SetBaseByName(player, "ER_THRES_IMMUNITY_AVG", sheet.thImmunity);
		SetBaseByName(player, "ER_THRES_ROBUSTNESS_AVG", sheet.thRobustness);
		SetBaseByName(player, "ER_THRES_FOCUS_AVG", sheet.thFocus);
		SetBaseByName(player, "ER_THRES_VITALITY_AVG", sheet.thVitality);
		SetBaseByName(player, "ER_THRES_MADNESS_AVG", sheet.thMadness);

		SetBaseByName(player, "ER_EQUIPLOAD_MAX_AVG", sheet.equipLoadMax);
		SetBaseByName(player, "ER_EQUIPLOAD_LIGHT_AVG", sheet.equipLoadLight);
		SetBaseByName(player, "ER_EQUIPLOAD_MEDIUM_AVG", sheet.equipLoadMedium);
		SetBaseByName(player, "ER_EQUIPLOAD_HEAVY_AVG", sheet.equipLoadHeavy);
	}
}

