#include "pch.h"

#include "Attributes.h"
#include "DerivedStats.h"
#include "PerkProgression.h"
#include "Serialization.h"

namespace
{
	constexpr auto kScriptName = "ERLS";

	std::int32_t ClampAttr(std::int32_t v)
	{
		// Keep it permissive; derived curves handle softcaps.
		return std::clamp(v, 0, 999);
	}

	std::int32_t ClampLevel(std::int32_t v)
	{
		return std::clamp(v, 1, 9999);
	}

	void RecomputeAndApplyNow()
	{
		const auto snapshot = ER::GetCurrentStatsSnapshot();
		ER::ApplyToPlayer(snapshot.derived, snapshot.attrs, snapshot.erLevel);
		ER::SyncPerkProgressionFromERLevel();
	}

	// Papyrus: bool Function SetERLevel(int level, bool applyNow = true) global native
	bool SetERLevel(RE::StaticFunctionTag*, std::int32_t level, bool applyNow)
	{
		Persist::SetERLevel(ClampLevel(level));
		if (applyNow) {
			RecomputeAndApplyNow();
		}
		return true;
	}

	// Papyrus: bool Function SetAttributes(int vig, int mnd, int end, int str, int dex, int intl, int fth, int arc, bool applyNow = true) global native
	bool SetAttributes(
		RE::StaticFunctionTag*,
		std::int32_t vig,
		std::int32_t mnd,
		std::int32_t end,
		std::int32_t str,
		std::int32_t dex,
		std::int32_t intl,
		std::int32_t fth,
		std::int32_t arc,
		bool applyNow)
	{
		ER::AttributeSet attrs{};
		attrs.vig = ClampAttr(vig);
		attrs.mnd = ClampAttr(mnd);
		attrs.end = ClampAttr(end);
		attrs.str = ClampAttr(str);
		attrs.dex = ClampAttr(dex);
		attrs.intl = ClampAttr(intl);
		attrs.fth = ClampAttr(fth);
		attrs.arc = ClampAttr(arc);
		Persist::SetAttributes(attrs);

		// Keep ER level consistent with “1 level per point spent” model.
		const auto spent = ER::PointsSpent(attrs);
		Persist::SetERLevel(std::max(1, spent));

		if (applyNow) {
			RecomputeAndApplyNow();
		}
		return true;
	}

	// Papyrus: bool Function SetAttributesAndLevel(int level, int vig, int mnd, int end, int str, int dex, int intl, int fth, int arc, bool applyNow = true) global native
	bool SetAttributesAndLevel(
		RE::StaticFunctionTag*,
		std::int32_t level,
		std::int32_t vig,
		std::int32_t mnd,
		std::int32_t end,
		std::int32_t str,
		std::int32_t dex,
		std::int32_t intl,
		std::int32_t fth,
		std::int32_t arc,
		bool applyNow)
	{
		ER::AttributeSet attrs{};
		attrs.vig = ClampAttr(vig);
		attrs.mnd = ClampAttr(mnd);
		attrs.end = ClampAttr(end);
		attrs.str = ClampAttr(str);
		attrs.dex = ClampAttr(dex);
		attrs.intl = ClampAttr(intl);
		attrs.fth = ClampAttr(fth);
		attrs.arc = ClampAttr(arc);
		Persist::SetAttributes(attrs);
		Persist::SetERLevel(ClampLevel(level));

		if (applyNow) {
			RecomputeAndApplyNow();
		}
		return true;
	}

	// Papyrus: bool Function ApplyNow() global native
	bool ApplyNow(RE::StaticFunctionTag*)
	{
		RecomputeAndApplyNow();
		return true;
	}
}

namespace ER::Papyrus
{
	bool Register()
	{
		auto* papyrus = SKSE::GetPapyrusInterface();
		if (!papyrus) {
			logger::warn("Papyrus interface not available");
			return false;
		}

		const auto ok = papyrus->Register([](RE::BSScript::IVirtualMachine* vm) {
			if (!vm) {
				return false;
			}

			vm->RegisterFunction("SetERLevel", kScriptName, SetERLevel);
			vm->RegisterFunction("SetAttributes", kScriptName, SetAttributes);
			vm->RegisterFunction("SetAttributesAndLevel", kScriptName, SetAttributesAndLevel);
			vm->RegisterFunction("ApplyNow", kScriptName, ApplyNow);

			logger::info("Papyrus API registered (script '{}')", kScriptName);
			return true;
		});

		if (!ok) {
			logger::warn("Papyrus registration failed");
		}
		return ok;
	}
}

