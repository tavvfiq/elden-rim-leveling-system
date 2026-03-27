#include "Economy.h"

#include "pch.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>

extern "C" bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse);

namespace ER
{
	std::int32_t GoldCostToLevelUp(std::int32_t fromLevel)
	{
		// Elden Ring rune cost to go from current level L to L+1.
		// Spec: leveling_curve.md in this repo.
		//
		//   x = max(0, (L - 11) * 0.02)
		//   cost = floor((x + 0.1) * (L + 81)^2) + 1
		//
		// L is the player's current level before spending (same as Persist::GetERLevel()).
		fromLevel = std::max(1, fromLevel);
		const double L = static_cast<double>(fromLevel);
		const double x = std::max(0.0, (L - 11.0) * 0.02);
		const double sum = L + 81.0;
		const double inner = (x + 0.1) * sum * sum;
		const auto floored = static_cast<std::int64_t>(std::floor(inner));
		const auto cost = floored + 1;
		return static_cast<std::int32_t>(std::min<std::int64_t>(cost, static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max())));
	}

	std::int32_t GetPlayerGold()
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return 0;
		}

		auto* gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000F);
		if (!gold) {
			return 0;
		}

		return static_cast<std::int32_t>(player->GetItemCount(gold));
	}

	bool TrySpendPlayerGold(std::int32_t amount)
	{
		if (amount <= 0) {
			return true;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return false;
		}

		auto* gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000F);
		if (!gold) {
			return false;
		}

		const auto current = static_cast<std::int32_t>(player->GetItemCount(gold));
		if (current < amount) {
			return false;
		}

		player->RemoveItem(gold, amount, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
		return true;
	}

	namespace
	{
		struct GoldDropConfig
		{
			float baseMultiplier{ 5.0f };
			float healthWeight{ 0.15f };
			float defaultTierMultiplier{ 1.0f };
			float contextMultiplier{ 1.0f };
			std::vector<std::pair<std::string, float>> keywordMultipliers{
				{ "ActorTypeDragon", 10.0f },
				{ "ActorTypeGiant", 2.5f }
			};
		};

		GoldDropConfig g_goldDropConfig{};

		std::filesystem::path GetGoldDropConfigPath()
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
			return dll.parent_path() / (dll.stem().string() + "_gold_kill.json");
		}

		void LoadGoldDropConfig()
		{
			g_goldDropConfig = GoldDropConfig{};
			const auto path = GetGoldDropConfigPath();
			if (path.empty()) {
				logger::warn("Gold kill config path resolve failed; using defaults");
				return;
			}

			std::ifstream in(path);
			if (!in) {
				logger::info("Gold kill config not found at {}; using defaults", path.string());
				return;
			}

			try {
				nlohmann::json j;
				in >> j;
				if (j.contains("baseMultiplier")) g_goldDropConfig.baseMultiplier = j["baseMultiplier"].get<float>();
				if (j.contains("healthWeight")) g_goldDropConfig.healthWeight = j["healthWeight"].get<float>();
				if (j.contains("defaultTierMultiplier")) g_goldDropConfig.defaultTierMultiplier = j["defaultTierMultiplier"].get<float>();
				if (j.contains("contextMultiplier")) g_goldDropConfig.contextMultiplier = j["contextMultiplier"].get<float>();

				// Back-compat with older fields if users still have them.
				if (j.contains("standardTierMultiplier")) g_goldDropConfig.defaultTierMultiplier = j["standardTierMultiplier"].get<float>();
				const float legacyElite = j.contains("eliteTierMultiplier") ? j["eliteTierMultiplier"].get<float>() : 2.5f;
				const float legacyBoss = j.contains("bossTierMultiplier") ? j["bossTierMultiplier"].get<float>() : 10.0f;

				if (j.contains("keywordMultipliers") && j["keywordMultipliers"].is_object()) {
					g_goldDropConfig.keywordMultipliers.clear();
					for (auto it = j["keywordMultipliers"].begin(); it != j["keywordMultipliers"].end(); ++it) {
						if (it.value().is_number()) {
							g_goldDropConfig.keywordMultipliers.emplace_back(it.key(), it.value().get<float>());
						}
					}
				} else {
					g_goldDropConfig.keywordMultipliers = {
						{ "ActorTypeDragon", legacyBoss },
						{ "ActorTypeGiant", legacyElite }
					};
				}

				g_goldDropConfig.baseMultiplier = std::max(0.0f, g_goldDropConfig.baseMultiplier);
				g_goldDropConfig.healthWeight = std::max(0.0f, g_goldDropConfig.healthWeight);
				g_goldDropConfig.defaultTierMultiplier = std::max(0.0f, g_goldDropConfig.defaultTierMultiplier);
				g_goldDropConfig.contextMultiplier = std::max(0.0f, g_goldDropConfig.contextMultiplier);
				for (auto& entry : g_goldDropConfig.keywordMultipliers) {
					entry.second = std::max(0.0f, entry.second);
				}

				logger::info(
					"Gold kill config loaded: baseMultiplier={} healthWeight={} defaultTier={} context={} keywordEntries={}",
					g_goldDropConfig.baseMultiplier,
					g_goldDropConfig.healthWeight,
					g_goldDropConfig.defaultTierMultiplier,
					g_goldDropConfig.contextMultiplier,
					g_goldDropConfig.keywordMultipliers.size());
			} catch (const std::exception& e) {
				logger::warn("Gold kill config parse failed: {}; using defaults", e.what());
				g_goldDropConfig = GoldDropConfig{};
			}
		}

		float ResolveTierMultiplier(RE::Actor* actor)
		{
			if (!actor) {
				return g_goldDropConfig.defaultTierMultiplier;
			}
			for (const auto& [keyword, mult] : g_goldDropConfig.keywordMultipliers) {
				if (!keyword.empty() && actor->HasKeywordString(keyword)) {
					return mult;
				}
			}
			return g_goldDropConfig.defaultTierMultiplier;
		}

		std::int32_t ComputeGoldFromKill(RE::Actor* victim)
		{
			if (!victim) {
				return 0;
			}
			const auto level = static_cast<std::int32_t>(victim->GetLevel());
			const float hp = victim->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kHealth);

			const float baseGold =
				static_cast<float>(std::max(1, level)) * g_goldDropConfig.baseMultiplier +
				std::max(1.0f, hp) * g_goldDropConfig.healthWeight;
			const float total = baseGold * ResolveTierMultiplier(victim) * g_goldDropConfig.contextMultiplier;
			return std::max(1, static_cast<std::int32_t>(std::lround(total)));
		}

		class GoldKillRewardSink final : public RE::BSTEventSink<RE::TESDeathEvent>
		{
		public:
			static GoldKillRewardSink* GetSingleton()
			{
				static GoldKillRewardSink singleton;
				return std::addressof(singleton);
			}

			RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override
			{
				if (!a_event || !a_event->dead || !a_event->actorDying || !a_event->actorKiller) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* player = RE::PlayerCharacter::GetSingleton();
				auto* killerRef = a_event->actorKiller.get();
				auto* victimRef = a_event->actorDying.get();
				if (!player || !killerRef || !victimRef) {
					return RE::BSEventNotifyControl::kContinue;
				}
				if (killerRef != player) {
					return RE::BSEventNotifyControl::kContinue;
				}
				if (victimRef->GetFormType() != RE::FormType::ActorCharacter) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* victim = static_cast<RE::Actor*>(victimRef);
				const auto reward = ComputeGoldFromKill(victim);
				if (reward <= 0) {
					return RE::BSEventNotifyControl::kContinue;
				}

				auto* gold = RE::TESForm::LookupByID<RE::TESObjectMISC>(0x0000000F);
				if (!gold) {
					return RE::BSEventNotifyControl::kContinue;
				}
				player->AddObjectToContainer(gold, nullptr, reward, nullptr);
				logger::info("Kill gold reward: victim={} level={} hp_base={} reward={}",
					victim->GetName(),
					victim->GetLevel(),
					victim->AsActorValueOwner()->GetBaseActorValue(RE::ActorValue::kHealth),
					reward);
				return RE::BSEventNotifyControl::kContinue;
			}
		};
	}

	void InstallGoldKillReward()
	{
		LoadGoldDropConfig();
		auto* sourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!sourceHolder) {
			logger::warn("Gold kill reward: ScriptEventSourceHolder unavailable");
			return;
		}
		sourceHolder->AddEventSink<RE::TESDeathEvent>(GoldKillRewardSink::GetSingleton());
		logger::info("Installed gold kill reward sink");
	}
}

