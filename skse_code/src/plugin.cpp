#include "Config.h"
#include "Economy.h"
#include "Hooks.h"
#include "Log.h"
#include "Prisma.h"
#include "Serialization.h"

#include "pch.h"

using namespace std::literals;

namespace
{
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
	__declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
	{
		SKSE::Init(skse);
		Log::Init();

		logger::info("AspectsAttributes: SKSEPlugin_Load");

		ER::Config::Load();

		Persist::Install();

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

