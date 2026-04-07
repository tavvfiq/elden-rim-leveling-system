#pragma once

namespace ER::Config
{
	// Load `<plugin_stem>.ini` from `Data/SKSE/Plugins/` (same folder as this DLL).
	// Safe to call once at plugin load; missing file or keys keep defaults.
	void Load();

	// Equip-load tier cutoffs as fractions of max computed equip-load.
	float EquipLoadLightFraction() noexcept;
	float EquipLoadMediumFraction() noexcept;

	// Legacy MCM parity (defaults match former MCM Helper sliders; see eras.ini [Tuning]).
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
	bool OverridePlayerGetLevel() noexcept;
	bool DisableVanillaXPGain() noexcept;
	bool EnableGoldKillDrops() noexcept;
	bool EnablePerkPointParity() noexcept;
	bool EnablePerkAutoUnlock() noexcept;
	// Humanoid NPCs (ActorTypeNPC): set Health/Magicka/Stamina/CarryWeight base from ER-derived curves once per load.
	bool ApplyNpcDerivedPoolsOnLoad() noexcept;

	// When true, opening the Prisma skill view on bed sleep is automatic.
	bool OpenSkillMenuOnSleep() noexcept;
	// When true, attribute allocation / confirm / gold level-up actions only work while the sleep session is active.
	bool RequireSleepForAttributeAllocation() noexcept;
	// When true, TESSleepStart fully restores current Health and Magicka (not Stamina).
	bool FullRestoreHealthMagickaOnSleep() noexcept;
}
