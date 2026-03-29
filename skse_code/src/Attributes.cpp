#include "Attributes.h"

#include "Serialization.h"
#include "pch.h"

namespace
{
	constexpr std::int32_t kNpcAttrClampMax = 99;

	float SkillBase(RE::Actor* actor, RE::ActorValue av)
	{
		if (!actor) {
			return 0.0f;
		}
		auto* owner = actor->AsActorValueOwner();
		if (!owner) {
			return 0.0f;
		}
		return owner->GetBaseActorValue(av);
	}

	std::int32_t ClampNpcAttr(float value)
	{
		const auto r = static_cast<std::int32_t>(std::lround(value));
		return std::clamp(r, 0, kNpcAttrClampMax);
	}

	ER::AttributeSet ComputeNPCAttributesFromVanillaSkills(RE::Actor* actor)
	{
		ER::AttributeSet out{};

		const float two = SkillBase(actor, RE::ActorValue::kTwoHanded);
		const float heavy = SkillBase(actor, RE::ActorValue::kHeavyArmor);
		const float smith = SkillBase(actor, RE::ActorValue::kSmithing);
		out.str = ClampNpcAttr((two + heavy + smith) / 3.0f);

		const float one = SkillBase(actor, RE::ActorValue::kOneHanded);
		const float arch = SkillBase(actor, RE::ActorValue::kArchery);
		const float sneak = SkillBase(actor, RE::ActorValue::kSneak);
		out.dex = ClampNpcAttr((one + arch + sneak) / 3.0f);

		const float dest = SkillBase(actor, RE::ActorValue::kDestruction);
		const float alt = SkillBase(actor, RE::ActorValue::kAlteration);
		out.intl = ClampNpcAttr((dest + alt) / 2.0f);

		const float rest = SkillBase(actor, RE::ActorValue::kRestoration);
		const float conj = SkillBase(actor, RE::ActorValue::kConjuration);
		out.fth = ClampNpcAttr((rest + conj) / 2.0f);

		const float alc = SkillBase(actor, RE::ActorValue::kAlchemy);
		const float pick = SkillBase(actor, RE::ActorValue::kPickpocket);
		const float ill = SkillBase(actor, RE::ActorValue::kIllusion);
		out.arc = ClampNpcAttr((alc + pick + ill) / 3.0f);

		const std::int32_t level = std::max(1, static_cast<std::int32_t>(actor->GetLevel()));
		const float levelHalf = 0.5f * static_cast<float>(level);

		const float block = SkillBase(actor, RE::ActorValue::kBlock);
		out.vig = ClampNpcAttr(10.0f + levelHalf + 0.5f * block);

		const float light = SkillBase(actor, RE::ActorValue::kLightArmor);
		out.end = ClampNpcAttr(10.0f + levelHalf + 0.5f * light);

		const float ench = SkillBase(actor, RE::ActorValue::kEnchanting);
		const float speech = SkillBase(actor, RE::ActorValue::kSpeech);
		out.mnd = ClampNpcAttr(10.0f + levelHalf + (ench + speech) / 4.0f);

		return out;
	}
}

namespace ER
{
	bool IsNPCAttributeEligible(RE::Actor* actor) noexcept
	{
		if (!actor) {
			return false;
		}
		if (actor->IsPlayerRef()) {
			return false;
		}
		return actor->HasKeywordString(kNPCDerivedAttributeKeyword);
	}

	std::int32_t GetActorEffectiveLevel(RE::Actor* actor) noexcept
	{
		if (!actor) {
			return 1;
		}
		if (actor->IsPlayerRef()) {
			return std::max(1, Persist::GetERLevel());
		}
		return std::max(1, static_cast<std::int32_t>(actor->GetLevel()));
	}

	RE::ActorValue ResolveActorValue(Attribute a)
	{
		(void)a;
		return RE::ActorValue::kNone;
	}

	std::int32_t Get(RE::Actor* actor, Attribute a)
	{
		const auto attrs = GetAll(actor);
		switch (a) {
		case Attribute::VIG: return attrs.vig;
		case Attribute::MND: return attrs.mnd;
		case Attribute::END: return attrs.end;
		case Attribute::STR: return attrs.str;
		case Attribute::DEX: return attrs.dex;
		case Attribute::INT: return attrs.intl;
		case Attribute::FTH: return attrs.fth;
		case Attribute::ARC: return attrs.arc;
		}
		return 0;
	}

	void Set(RE::Actor* actor, Attribute a, std::int32_t value)
	{
		if (!actor || !actor->IsPlayerRef()) {
			return;
		}
		auto attrs = Persist::GetAttributes();
		value = std::max(0, value);
		switch (a) {
		case Attribute::VIG: attrs.vig = value; break;
		case Attribute::MND: attrs.mnd = value; break;
		case Attribute::END: attrs.end = value; break;
		case Attribute::STR: attrs.str = value; break;
		case Attribute::DEX: attrs.dex = value; break;
		case Attribute::INT: attrs.intl = value; break;
		case Attribute::FTH: attrs.fth = value; break;
		case Attribute::ARC: attrs.arc = value; break;
		}
		Persist::SetAttributes(attrs);
	}

	AttributeSet GetAll(RE::Actor* actor)
	{
		if (!actor) {
			return {};
		}
		if (actor->IsPlayerRef()) {
			return Persist::GetAttributes();
		}
		if (IsNPCAttributeEligible(actor)) {
			return ComputeNPCAttributesFromVanillaSkills(actor);
		}
		return {};
	}

	std::int32_t PointsSpent(const AttributeSet& set)
	{
		// Baselines will come later; for now treat current totals as “spent”.
		return set.vig + set.mnd + set.end + set.str + set.dex + set.intl + set.fth + set.arc;
	}

	std::int32_t ERLevelFromPointsSpent(std::int32_t spent)
	{
		return std::max(1, 1 + spent);
	}
}
