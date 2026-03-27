#include "Attributes.h"

#include "Serialization.h"
#include "pch.h"

namespace ER
{
	RE::ActorValue ResolveActorValue(Attribute a)
	{
		(void)a;
		return RE::ActorValue::kNone;
	}

	std::int32_t Get(RE::Actor* actor, Attribute a)
	{
		(void)actor;
		const auto attrs = Persist::GetAttributes();
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
		(void)actor;
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
		(void)actor;
		return Persist::GetAttributes();
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

