#include "Config.h"
#include "DerivedStats.h"
#include "ERLS_API.h"
#include "Economy.h"
#include "Hooks.h"
#include "Log.h"
#include "PapyrusAPI.h"
#include "PerkProgression.h"
#include "Prisma.h"
#include "Serialization.h"

#include "pch.h"

using namespace std::literals;

namespace
{
	bool GetPlayerStats_API(ERLS_API::PlayerStatsSnapshot* outSnapshot) noexcept
	{
		if (!outSnapshot) {
			return false;
		}
		const auto snapshot = ER::GetCurrentStatsSnapshot();
		outSnapshot->erLevel = snapshot.erLevel;
		outSnapshot->attrs = {
			snapshot.attrs.vig,
			snapshot.attrs.mnd,
			snapshot.attrs.end,
			snapshot.attrs.str,
			snapshot.attrs.dex,
			snapshot.attrs.intl,
			snapshot.attrs.fth,
			snapshot.attrs.arc
		};
		outSnapshot->derived = {
			snapshot.derived.maxHP,
			snapshot.derived.maxMP,
			snapshot.derived.maxSP,
			snapshot.derived.carryWeight
		};
		outSnapshot->defense = {
			snapshot.sheet.l1Phys,
			snapshot.sheet.l1Magic,
			snapshot.sheet.l1Fire,
			snapshot.sheet.l1Lightning,
			snapshot.sheet.l1Frost,
			snapshot.sheet.l1Poison
		};
		outSnapshot->thresholds = {
			snapshot.sheet.thImmunity,
			snapshot.sheet.thRobustness,
			snapshot.sheet.thFocus,
			snapshot.sheet.thVitality,
			snapshot.sheet.thMadness
		};
		outSnapshot->equipLoad = {
			snapshot.sheet.equipLoadMax,
			snapshot.sheet.equipLoadLight,
			snapshot.sheet.equipLoadMedium,
			snapshot.sheet.equipLoadHeavy
		};
		return true;
	}

	std::int32_t GetERLevel_API() noexcept
	{
		return Persist::GetERLevel();
	}

	bool GetAttributes_API(ERLS_API::AttributeSet* outAttrs) noexcept
	{
		if (!outAttrs) {
			return false;
		}
		const auto attrs = Persist::GetAttributes();
		*outAttrs = {
			attrs.vig,
			attrs.mnd,
			attrs.end,
			attrs.str,
			attrs.dex,
			attrs.intl,
			attrs.fth,
			attrs.arc
		};
		return true;
	}

	ERLS_API::IERLS1 g_apiV1{
		&GetPlayerStats_API,
		&GetERLevel_API,
		&GetAttributes_API
	};

	void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
	{
		if (!message) {
			return;
		}

		switch (message->type) {
		case SKSE::MessagingInterface::kPostLoad:
			logger::info("SKSE message: kPostLoad");
			Prisma::Install();
			break;
		case SKSE::MessagingInterface::kDataLoaded:
			logger::info("SKSE message: kDataLoaded");
			ER::SyncPerkProgressionFromERLevel();
			if (ER::Config::EnableGoldKillDrops()) {
				ER::InstallGoldKillReward();
			}
			break;
		case SKSE::MessagingInterface::kPostPostLoad:
			logger::info("SKSE message: kPostPostLoad");
			break;
		default:
			break;
		}
	}
}

extern "C"
{
	__declspec(dllexport) void* RequestPluginAPI(ERLS_API::InterfaceVersion interfaceVersion)
	{
		switch (interfaceVersion) {
		case ERLS_API::InterfaceVersion::V1:
			return std::addressof(g_apiV1);
		default:
			return nullptr;
		}
	}

	__declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
	{
		SKSE::Init(skse);
		Log::Init();

		logger::info("AspectsAttributes: SKSEPlugin_Load");

		ER::Config::Load();

		Persist::Install();
		ER::Papyrus::Register();

		auto* messaging = SKSE::GetMessagingInterface();
		if (messaging) {
			messaging->RegisterListener(OnSKSEMessage);
		} else {
			logger::warn("No SKSE messaging interface");
		}

		Hooks::Install();
		return true;
	}
}

