#include "pch.h"

#include "Config.h"
#include "PerkProgression.h"
#include "Serialization.h"

#include <filesystem>
#include <fstream>
#include <string_view>

extern "C" bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse);

namespace
{
	constexpr std::int32_t kMaxPerkPoints = 127;

	struct PerkUnlockRule
	{
		// Preferred: modName + local formId (load-order independent).
		// Fallback: editorId (may not be stable across overhauls).
		std::string modName;
		std::uint32_t formID{ 0 };  // local formID within mod (e.g. 0x00012345)
		std::string editorID;
		std::int32_t minERLevel{ 1 };
	};

	struct PerkUnlockConfig
	{
		std::vector<PerkUnlockRule> rules{
			// Smithing gates
			{ .editorID = "SteelSmithing", .minERLevel = 2 },
			{ .editorID = "ArcaneBlacksmith", .minERLevel = 10 },
			{ .editorID = "ElvenSmithing", .minERLevel = 12 },
			{ .editorID = "DwarvenSmithing", .minERLevel = 16 },
			{ .editorID = "OrcishSmithing", .minERLevel = 20 },
			{ .editorID = "EbonySmithing", .minERLevel = 30 },
			{ .editorID = "GlassSmithing", .minERLevel = 40 },
			{ .editorID = "DaedricSmithing", .minERLevel = 50 },
			{ .editorID = "DragonArmor", .minERLevel = 60 },
			// Enchanting gates
			{ .editorID = "Enchanter00", .minERLevel = 2 },
			{ .editorID = "Enchanter01", .minERLevel = 10 },
			{ .editorID = "Enchanter02", .minERLevel = 20 },
			{ .editorID = "Enchanter03", .minERLevel = 30 },
			{ .editorID = "Enchanter04", .minERLevel = 40 },
			{ .editorID = "InsightfulEnchanter", .minERLevel = 25 },
			{ .editorID = "CorpusEnchanter", .minERLevel = 30 },
			{ .editorID = "ExtraEffect", .minERLevel = 50 },
		};
	};

	PerkUnlockConfig g_unlockConfig{};
	bool g_unlockConfigLoaded{ false };

	std::filesystem::path GetPerkUnlockConfigPath()
	{
		wchar_t buffer[MAX_PATH]{};
		HMODULE module = nullptr;
		if (!GetModuleHandleExW(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				reinterpret_cast<LPCWSTR>(&SKSEPlugin_Load),
				&module) ||
			!module) {
			return {};
		}
		if (GetModuleFileNameW(module, buffer, MAX_PATH) == 0) {
			return {};
		}
		std::filesystem::path dll{ buffer };
		return dll.parent_path() / (dll.stem().string() + "_perk_unlocks.json");
	}

	void LoadPerkUnlockConfig()
	{
		g_unlockConfig = PerkUnlockConfig{};
		g_unlockConfigLoaded = true;

		const auto path = GetPerkUnlockConfigPath();
		if (path.empty()) {
			logger::warn("Perk unlock config path resolve failed; using defaults");
			return;
		}

		std::ifstream in(path);
		if (!in) {
			logger::info("Perk unlock config not found at {}; using defaults", path.string());
			return;
		}

		try {
			nlohmann::json j;
			in >> j;
			if (!j.contains("autoUnlockPerks") || !j["autoUnlockPerks"].is_array()) {
				logger::warn("Perk unlock config missing autoUnlockPerks array; using defaults");
				return;
			}

			std::vector<PerkUnlockRule> parsed;
			for (const auto& entry : j["autoUnlockPerks"]) {
				if (!entry.is_object()) {
					continue;
				}
				const auto editorID = entry.value("editorId", std::string{});
				const auto modName = entry.value("modName", std::string{});
				const auto formIDStr = entry.value("formId", std::string{});
				const auto minERLevel = std::max(1, entry.value("minERLevel", 1));
				if (editorID.empty() && (modName.empty() || formIDStr.empty())) {
					continue;
				}

				std::uint32_t localFormID = 0;
				if (!modName.empty() && !formIDStr.empty()) {
					try {
						std::string s = formIDStr;
						// Accept "0x..." or raw hex/decimal.
						std::size_t idx = 0;
						const int base = (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) ? 16 : 0;
						const auto v = std::stoul(s, &idx, base);
						if (idx == s.size()) {
							localFormID = static_cast<std::uint32_t>(v);
						}
					} catch (...) {
						localFormID = 0;
					}
				}

				PerkUnlockRule rule;
				rule.modName = modName;
				rule.formID = localFormID;
				rule.editorID = editorID;
				rule.minERLevel = minERLevel;
				parsed.push_back(std::move(rule));
			}

			if (!parsed.empty()) {
				g_unlockConfig.rules = std::move(parsed);
			}
			logger::info("Perk unlock config loaded: rules={}", g_unlockConfig.rules.size());
		} catch (const std::exception& e) {
			logger::warn("Perk unlock config parse failed: {}; using defaults", e.what());
			g_unlockConfig = PerkUnlockConfig{};
		}
	}

	void SyncPerkPointParityFromERLevel()
	{
		if (!ER::Config::EnablePerkPointParity()) {
			return;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		const auto erLevel = std::max(1, Persist::GetERLevel());
		const auto lastSyncedLevel = std::max(1, Persist::GetPerkSyncLevel());
		if (erLevel <= lastSyncedLevel) {
			if (erLevel < lastSyncedLevel) {
				// ER level moved backwards (respec/debug). Track new baseline, do not remove perk points.
				Persist::SetPerkSyncLevel(erLevel);
			}
			return;
		}

		const auto levelsGained = erLevel - lastSyncedLevel;
		auto& gameState = player->GetGameStatsData();
		const auto currentPerkPoints = static_cast<std::int32_t>(gameState.perkCount);
		const auto newPerkPoints = std::clamp(currentPerkPoints + levelsGained, 0, kMaxPerkPoints);
		const auto actuallyAdded = newPerkPoints - currentPerkPoints;

		gameState.perkCount = static_cast<std::int8_t>(newPerkPoints);
		Persist::SetPerkSyncLevel(erLevel);

		logger::info(
			"Perk parity sync: erLevel={} lastSynced={} gained={} perkPoints {}->{} (added={})",
			erLevel,
			lastSyncedLevel,
			levelsGained,
			currentPerkPoints,
			newPerkPoints,
			actuallyAdded);
	}

	void SyncPerkAutoUnlockFromERLevel()
	{
		if (!ER::Config::EnablePerkAutoUnlock()) {
			return;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		if (!g_unlockConfigLoaded) {
			LoadPerkUnlockConfig();
		}

		const auto erLevel = std::max(1, Persist::GetERLevel());
		std::size_t unlockedCount = 0;
		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		for (const auto& rule : g_unlockConfig.rules) {
			if (erLevel < rule.minERLevel) {
				continue;
			}

			RE::BGSPerk* perk = nullptr;
			if (dataHandler && !rule.modName.empty() && rule.formID != 0) {
				perk = dataHandler->LookupForm<RE::BGSPerk>(rule.formID, rule.modName);
				if (!perk) {
					logger::warn(
						"Perk auto-unlock: perk not found by form (modName='{}', formId=0x{:08X})",
						rule.modName,
						rule.formID);
				}
			}
			if (!perk && !rule.editorID.empty()) {
				perk = RE::TESForm::LookupByEditorID<RE::BGSPerk>(rule.editorID);
			}
			if (!perk) {
				logger::warn(
					"Perk auto-unlock: perk not found (editorId='{}', modName='{}', formId=0x{:08X})",
					rule.editorID,
					rule.modName,
					rule.formID);
				continue;
			}

			if (player->HasPerk(perk)) {
				continue;
			}

			player->AddPerk(perk, 0);
			++unlockedCount;
			logger::info("Perk auto-unlock: granted '{}' at ER level {}", rule.editorID, erLevel);
		}

		if (unlockedCount > 0) {
			logger::info("Perk auto-unlock sync complete: {} perk(s) granted", unlockedCount);
		}
	}
}

namespace ER
{
	void SyncPerkProgressionFromERLevel()
	{
		SyncPerkPointParityFromERLevel();
		SyncPerkAutoUnlockFromERLevel();
	}
}

