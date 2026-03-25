#include "Attributes.h"

#include "pch.h"

namespace ER
{
	RE::ActorValue ResolveActorValue(Attribute a)
	{
		const auto name = ToAVName(a);
		if (name.empty()) {
			return RE::ActorValue::kNone;
		}

		auto* list = RE::ActorValueList::GetSingleton();
		if (!list) {
			return RE::ActorValue::kNone;
		}

		return list->LookupActorValueByName(name);
	}

	std::int32_t Get(RE::Actor* actor, Attribute a)
	{
		if (!actor) {
			return 0;
		}

		const auto av = ResolveActorValue(a);
		if (av == RE::ActorValue::kNone) {
			return 0;
		}

		const float value = actor->AsActorValueOwner()->GetBaseActorValue(av);
		return static_cast<std::int32_t>(std::lround(value));
	}

	void Set(RE::Actor* actor, Attribute a, std::int32_t value)
	{
		if (!actor) {
			return;
		}

		const auto av = ResolveActorValue(a);
		if (av == RE::ActorValue::kNone) {
			return;
		}

		actor->AsActorValueOwner()->SetBaseActorValue(av, static_cast<float>(value));
	}

	AttributeSet GetAll(RE::Actor* actor)
	{
		AttributeSet set;
		set.vig = Get(actor, Attribute::VIG);
		set.mnd = Get(actor, Attribute::MND);
		set.end = Get(actor, Attribute::END);
		set.str = Get(actor, Attribute::STR);
		set.dex = Get(actor, Attribute::DEX);
		set.intl = Get(actor, Attribute::INT);
		set.fth = Get(actor, Attribute::FTH);
		set.arc = Get(actor, Attribute::ARC);
		return set;
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

