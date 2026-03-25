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

		// Publish “AVG” derived values for the combat framework to read directly from ActorValues.
		// Spec source: derived_stats_map.md + combat framework requirement.md
		//
		// NOTE: these are intentionally stored as base AVs (not temp modifiers) so other plugins can query them
		// without needing to call into this plugin.
		const auto attrs = ER::GetAll(player);
		const std::int32_t level = std::max(1, Persist::GetERLevel());

		// Layer 1 (flat defense buckets)
		const float l1_phys =
			static_cast<float>(level) +
			static_cast<float>(attrs.str) * 0.5f +
			static_cast<float>(attrs.end) * 0.2f;

		const float l1_magic =
			static_cast<float>(level) +
			static_cast<float>(attrs.intl) * 0.6f +
			static_cast<float>(attrs.mnd) * 0.1f;

		const float l1_fire =
			static_cast<float>(level) +
			static_cast<float>(attrs.vig) * 0.6f;

		const float l1_lightning =
			static_cast<float>(level) +
			static_cast<float>(attrs.fth) * 0.4f;

		// For now: frost/poison share the same “magic-like” Layer 1 bucket.
		const float l1_frost = l1_magic;
		const float l1_poison = l1_magic;

		SetBaseByName(player, "ER_L1DEF_PHYS_AVG", l1_phys);
		SetBaseByName(player, "ER_L1DEF_MAGIC_AVG", l1_magic);
		SetBaseByName(player, "ER_L1DEF_FIRE_AVG", l1_fire);
		SetBaseByName(player, "ER_L1DEF_FROST_AVG", l1_frost);
		SetBaseByName(player, "ER_L1DEF_POISON_AVG", l1_poison);
		SetBaseByName(player, "ER_L1DEF_LIGHTNING_AVG", l1_lightning);

		// Status thresholds (Layer 3)
		const float th_immunity = static_cast<float>(level + attrs.vig);
		const float th_robustness = static_cast<float>(level + attrs.end);
		const float th_focus = static_cast<float>(level + attrs.mnd);
		const float th_vitality = static_cast<float>(level + attrs.arc);
		const float th_madness = static_cast<float>(stats.maxHP + stats.maxMP) * 0.5f;

		SetBaseByName(player, "ER_THRES_IMMUNITY_AVG", th_immunity);
		SetBaseByName(player, "ER_THRES_ROBUSTNESS_AVG", th_robustness);
		SetBaseByName(player, "ER_THRES_FOCUS_AVG", th_focus);
		SetBaseByName(player, "ER_THRES_VITALITY_AVG", th_vitality);
		SetBaseByName(player, "ER_THRES_MADNESS_AVG", th_madness);

		// Equip-load thresholds (END-driven). ERCF uses these for movement/dodge tier logic.
		// We publish them as derived from our computed carry weight to avoid reading volatile armor/perk buffs.
		const float equipMax = static_cast<float>(stats.carryWeight);
		const float lightFrac = ER::Config::EquipLoadLightFraction();
		const float mediumFrac = ER::Config::EquipLoadMediumFraction();
		SetBaseByName(player, "ER_EQUIPLOAD_MAX_AVG", equipMax);
		SetBaseByName(player, "ER_EQUIPLOAD_LIGHT_AVG", equipMax * lightFrac);
		SetBaseByName(player, "ER_EQUIPLOAD_MEDIUM_AVG", equipMax * mediumFrac);
		SetBaseByName(player, "ER_EQUIPLOAD_HEAVY_AVG", equipMax * 1.00f);
	}
}

