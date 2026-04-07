#include "pch.h"

#include "Config.h"
#include "Log.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

extern "C" bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse);

namespace
{
	struct Settings
	{
		float equipLoadLight{ 0.30f };
		float equipLoadMedium{ 0.70f };

		float hpPerLevelGain{ 0.25f };
		float mpPerLevelGain{ 0.25f };
		float spPerLevelGain{ 0.25f };
		float bonusHp{ 5.0f };
		float bonusMp{ 5.0f };
		float bonusSp{ 5.0f };
		float carryWeightStr{ 15.0f };
		float carryWeightEnd{ 15.0f };
		float magicResistGain{ 1.5f };
		float diseaseResistGain{ 1.5f };
		float poisonResistGain{ 1.5f };
		bool overridePlayerGetLevel{ false };
		bool disableVanillaXPGain{ false };
		bool enableGoldKillDrops{ false };
		bool enablePerkPointParity{ true };
		bool enablePerkAutoUnlock{ true };
		bool applyNpcDerivedPoolsOnLoad{ true };
		bool openSkillMenuOnSleep{ true };
		bool requireSleepForAttributeAllocation{ true };
		bool fullRestoreHealthMagickaOnSleep{ true };
	};

	Settings g_settings{};

	std::filesystem::path GetPluginIniPath()
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
		return dll.parent_path() / (dll.stem().string() + ".ini");
	}

	void TrimInPlace(std::string& s)
	{
		while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
			s.erase(s.begin());
		}
		while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
			s.pop_back();
		}
	}

	std::string ToLower(std::string s)
	{
		for (auto& c : s) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
		return s;
	}

	bool ParseFloat(std::string_view sv, float& out)
	{
		std::string tmp{ sv };
		try {
			out = std::stof(tmp);
			return true;
		} catch (...) {
			return false;
		}
	}

	bool ParseBool(std::string_view sv, bool& out)
	{
		auto s = ToLower(std::string{ sv });
		TrimInPlace(s);
		if (s == "1" || s == "true" || s == "yes" || s == "on") {
			out = true;
			return true;
		}
		if (s == "0" || s == "false" || s == "no" || s == "off") {
			out = false;
			return true;
		}
		return false;
	}

	void ApplyKeyValue(const std::string& sectionLower, const std::string& keyLower, std::string value)
	{
		TrimInPlace(value);
		float f = 0.0f;
		bool b = false;

		if (sectionLower == "equpload") {
			if (keyLower == "lightfraction" || keyLower == "light") {
				if (ParseFloat(value, f)) {
					g_settings.equipLoadLight = f;
				}
			} else if (keyLower == "mediumfraction" || keyLower == "medium") {
				if (ParseFloat(value, f)) {
					g_settings.equipLoadMedium = f;
				}
			}
			return;
		}

		if (sectionLower == "sleep") {
			if (keyLower == "openskillmenuonsleep" && ParseBool(value, b)) {
				g_settings.openSkillMenuOnSleep = b;
			} else if (keyLower == "requiresleepforattributeallocation" && ParseBool(value, b)) {
				g_settings.requireSleepForAttributeAllocation = b;
			} else if (keyLower == "fullrestorehealthmagickaonsleep" && ParseBool(value, b)) {
				g_settings.fullRestoreHealthMagickaOnSleep = b;
			}
			return;
		}

		if (sectionLower != "tuning") {
			return;
		}

		if (keyLower == "overrideplayergetlevel" && ParseBool(value, b)) {
			g_settings.overridePlayerGetLevel = b;
		} else if (keyLower == "disablevanillaxpgain" && ParseBool(value, b)) {
			g_settings.disableVanillaXPGain = b;
		} else if (keyLower == "enablegoldkilldrops" && ParseBool(value, b)) {
			g_settings.enableGoldKillDrops = b;
		} else if (keyLower == "enableperkpointparity" && ParseBool(value, b)) {
			g_settings.enablePerkPointParity = b;
		} else if (keyLower == "enableperkautounlock" && ParseBool(value, b)) {
			g_settings.enablePerkAutoUnlock = b;
		} else if (keyLower == "applynpcderivedpoolsonload" && ParseBool(value, b)) {
			g_settings.applyNpcDerivedPoolsOnLoad = b;
		} else if (keyLower == "hpperlevelgain" && ParseFloat(value, f)) {
			g_settings.hpPerLevelGain = f;
		} else if (keyLower == "mpperlevelgain" && ParseFloat(value, f)) {
			g_settings.mpPerLevelGain = f;
		} else if (keyLower == "spperlevelgain" && ParseFloat(value, f)) {
			g_settings.spPerLevelGain = f;
		} else if (keyLower == "bonushp" && ParseFloat(value, f)) {
			g_settings.bonusHp = f;
		} else if (keyLower == "bonusmp" && ParseFloat(value, f)) {
			g_settings.bonusMp = f;
		} else if (keyLower == "bonussp" && ParseFloat(value, f)) {
			g_settings.bonusSp = f;
		} else if (keyLower == "carryweightperstr" && ParseFloat(value, f)) {
			g_settings.carryWeightStr = f;
		} else if (keyLower == "carryweightperend" && ParseFloat(value, f)) {
			g_settings.carryWeightEnd = f;
		} else if (keyLower == "magicresistperwill" && ParseFloat(value, f)) {
			g_settings.magicResistGain = f;
		} else if (keyLower == "diseaseresistperend" && ParseFloat(value, f)) {
			g_settings.diseaseResistGain = f;
		} else if (keyLower == "poisonresistperend" && ParseFloat(value, f)) {
			g_settings.poisonResistGain = f;
		}
	}

	void SanitizeEquipLoadFractions()
	{
		auto clamp01 = [](float x) { return std::clamp(x, 0.01f, 1.0f); };
		g_settings.equipLoadLight = clamp01(g_settings.equipLoadLight);
		g_settings.equipLoadMedium = clamp01(g_settings.equipLoadMedium);
		if (g_settings.equipLoadMedium <= g_settings.equipLoadLight) {
			logger::warn("Config: EquipLoad MediumFraction must be > LightFraction; using defaults 0.30 / 0.70");
			g_settings.equipLoadLight = 0.30f;
			g_settings.equipLoadMedium = 0.70f;
		}
	}
}

namespace ER::Config
{
	void Load()
	{
		g_settings = Settings{};

		const auto iniPath = GetPluginIniPath();
		if (iniPath.empty()) {
			logger::warn("Config: could not resolve plugin path; using defaults");
			SanitizeEquipLoadFractions();
			return;
		}

		std::ifstream in(iniPath);
		if (!in) {
			logger::info("Config: no INI at {} (using defaults)", iniPath.string());
			SanitizeEquipLoadFractions();
			return;
		}

		std::string line;
		std::string sectionLower = "default";

		while (std::getline(in, line)) {
			TrimInPlace(line);
			if (line.empty() || line[0] == '#' || line[0] == ';') {
				continue;
			}
			if (line.front() == '[' && line.back() == ']') {
				sectionLower = ToLower(line.substr(1, line.size() - 2));
				TrimInPlace(sectionLower);
				continue;
			}
			const auto eq = line.find('=');
			if (eq == std::string::npos) {
				continue;
			}
			std::string key = line.substr(0, eq);
			std::string val = line.substr(eq + 1);
			TrimInPlace(key);
			TrimInPlace(val);
			ApplyKeyValue(sectionLower, ToLower(key), val);
		}

		SanitizeEquipLoadFractions();
		logger::info("Config: loaded {}", iniPath.string());
	}

	float EquipLoadLightFraction() noexcept
	{
		return g_settings.equipLoadLight;
	}

	float EquipLoadMediumFraction() noexcept
	{
		return g_settings.equipLoadMedium;
	}

	float HpPerLevelGain() noexcept
	{
		return g_settings.hpPerLevelGain;
	}

	float MpPerLevelGain() noexcept
	{
		return g_settings.mpPerLevelGain;
	}

	float SpPerLevelGain() noexcept
	{
		return g_settings.spPerLevelGain;
	}

	float BonusHpPerAttrPoint() noexcept
	{
		return g_settings.bonusHp;
	}

	float BonusMpPerAttrPoint() noexcept
	{
		return g_settings.bonusMp;
	}

	float BonusSpPerAttrPoint() noexcept
	{
		return g_settings.bonusSp;
	}

	float CarryWeightPerStr() noexcept
	{
		return g_settings.carryWeightStr;
	}

	float CarryWeightPerEnd() noexcept
	{
		return g_settings.carryWeightEnd;
	}

	float MagicResistPerWill() noexcept
	{
		return g_settings.magicResistGain;
	}

	float DiseaseResistPerEnd() noexcept
	{
		return g_settings.diseaseResistGain;
	}

	float PoisonResistPerEnd() noexcept
	{
		return g_settings.poisonResistGain;
	}

	bool OverridePlayerGetLevel() noexcept
	{
		return g_settings.overridePlayerGetLevel;
	}

	bool DisableVanillaXPGain() noexcept
	{
		return g_settings.disableVanillaXPGain;
	}

	bool EnableGoldKillDrops() noexcept
	{
		return g_settings.enableGoldKillDrops;
	}

	bool EnablePerkPointParity() noexcept
	{
		return g_settings.enablePerkPointParity;
	}

	bool EnablePerkAutoUnlock() noexcept
	{
		return g_settings.enablePerkAutoUnlock;
	}

	bool ApplyNpcDerivedPoolsOnLoad() noexcept
	{
		return g_settings.applyNpcDerivedPoolsOnLoad;
	}

	bool OpenSkillMenuOnSleep() noexcept
	{
		return g_settings.openSkillMenuOnSleep;
	}

	bool RequireSleepForAttributeAllocation() noexcept
	{
		return g_settings.requireSleepForAttributeAllocation;
	}

	bool FullRestoreHealthMagickaOnSleep() noexcept
	{
		return g_settings.fullRestoreHealthMagickaOnSleep;
	}

}
