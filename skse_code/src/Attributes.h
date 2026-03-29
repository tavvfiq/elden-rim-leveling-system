#pragma once

namespace ER
{
	// Vanilla keyword: actors that use skill→ER mapping (see vanilla_skill_point_er_attribute.md).
	// Player is excluded; player attributes come from Persist.
	inline constexpr std::string_view kNPCDerivedAttributeKeyword = "ActorTypeNPC";

	[[nodiscard]] bool IsNPCAttributeEligible(RE::Actor* actor) noexcept;

	enum class Attribute : std::uint8_t
	{
		VIG,
		MND,
		END,
		STR,
		DEX,
		INT,
		FTH,
		ARC
	};

	struct AttributeSet
	{
		std::int32_t vig{ 0 };
		std::int32_t mnd{ 0 };
		std::int32_t end{ 0 };
		std::int32_t str{ 0 };
		std::int32_t dex{ 0 };
		std::int32_t intl{ 0 };
		std::int32_t fth{ 0 };
		std::int32_t arc{ 0 };
	};

	// Effective ER level for sheet math: player uses persisted ER level; everyone else uses Skyrim level (min 1).
	[[nodiscard]] std::int32_t GetActorEffectiveLevel(RE::Actor* actor) noexcept;

	// Resolved attributes: player from Persist; ActorTypeNPC (non-player) from vanilla skills; else zeros.
	[[nodiscard]] AttributeSet GetAll(RE::Actor* actor);

	// AV names. Must match what ActorValueData / AV Generator registers.
	constexpr std::string_view ToAVName(Attribute a) noexcept
	{
		switch (a) {
		case Attribute::VIG: return "ER_VIG";
		case Attribute::MND: return "ER_MND";
		case Attribute::END: return "ER_END";
		case Attribute::STR: return "ER_STR";
		case Attribute::DEX: return "ER_DEX";
		case Attribute::INT: return "ER_INT";
		case Attribute::FTH: return "ER_FTH";
		case Attribute::ARC: return "ER_ARC";
		default: return "";
		}
	}

	RE::ActorValue ResolveActorValue(Attribute a);
	std::int32_t Get(RE::Actor* actor, Attribute a);
	void Set(RE::Actor* actor, Attribute a, std::int32_t value);

	// Points model (initially no class templates; base is treated as 0 until we add baselines).
	std::int32_t PointsSpent(const AttributeSet& set);
	std::int32_t ERLevelFromPointsSpent(std::int32_t spent);
}

