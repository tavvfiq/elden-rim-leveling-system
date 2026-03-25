#pragma once

namespace ER::Config
{
	// Load `<plugin_stem>.ini` from `Data/SKSE/Plugins/` (same folder as this DLL).
	// Safe to call once at plugin load; missing file or keys keep defaults.
	void Load();

	// Equip-load tier cutoffs as fractions of ER_EQUIPLOAD_MAX_AVG (carry-weight proxy).
	float EquipLoadLightFraction() noexcept;
	float EquipLoadMediumFraction() noexcept;

	// Legacy MCM parity (defaults match former MCM Helper sliders; see eldenrimlevelingsystem.ini [Tuning]).
	// DerivedStats does not use all of these yet; ERCF or future curves can read via getters when wired.
	float HpPerLevelGain() noexcept;
	float MpPerLevelGain() noexcept;
	float SpPerLevelGain() noexcept;
	float BonusHpPerAttrPoint() noexcept;
	float BonusMpPerAttrPoint() noexcept;
	float BonusSpPerAttrPoint() noexcept;
	float CarryWeightPerStr() noexcept;
	float CarryWeightPerEnd() noexcept;
	float MagicResistPerWill() noexcept;
	float DiseaseResistPerEnd() noexcept;
	float PoisonResistPerEnd() noexcept;
}
