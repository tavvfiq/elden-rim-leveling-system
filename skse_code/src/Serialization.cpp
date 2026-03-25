#include "Serialization.h"

#include "pch.h"

namespace
{
	constexpr std::uint32_t kUniqueID = 'ASPA';
	constexpr std::uint32_t kRecord_Unspent = 'UNSP';
	constexpr std::uint32_t kRecord_ERLevel = 'ERLV';

	std::int32_t g_unspentPoints = 0;
	std::int32_t g_erLevel = 1;

	void OnSave(SKSE::SerializationInterface* intfc)
	{
		if (!intfc) {
			return;
		}

		intfc->OpenRecord(kRecord_Unspent, 1);
		intfc->WriteRecordData(&g_unspentPoints, sizeof(g_unspentPoints));

		intfc->OpenRecord(kRecord_ERLevel, 1);
		intfc->WriteRecordData(&g_erLevel, sizeof(g_erLevel));
	}

	void OnLoad(SKSE::SerializationInterface* intfc)
	{
		if (!intfc) {
			return;
		}

		std::uint32_t type = 0;
		std::uint32_t version = 0;
		std::uint32_t length = 0;

		while (intfc->GetNextRecordInfo(type, version, length)) {
			switch (type) {
			case kRecord_Unspent: {
				std::int32_t value = 0;
				if (intfc->ReadRecordData(&value, sizeof(value)) == sizeof(value)) {
					g_unspentPoints = value;
				}
				break;
			}
			case kRecord_ERLevel: {
				std::int32_t value = 1;
				if (intfc->ReadRecordData(&value, sizeof(value)) == sizeof(value)) {
					g_erLevel = std::max(1, value);
				}
				break;
			}
			default:
				// Skip unknown record
				intfc->SkipRecord();
				break;
			}
		}
	}

	void OnRevert(SKSE::SerializationInterface*)
	{
		g_unspentPoints = 0;
		g_erLevel = 1;
	}
}

namespace Persist
{
	void Install()
	{
		auto* ser = SKSE::GetSerializationInterface();
		if (!ser) {
			logger::warn("No SKSE serialization interface");
			return;
		}

		ser->SetUniqueID(kUniqueID);
		ser->SetSaveCallback(OnSave);
		ser->SetLoadCallback(OnLoad);
		ser->SetRevertCallback(OnRevert);

		logger::info("Serialization installed (unique id {})", kUniqueID);
	}

	std::int32_t GetUnspentPoints()
	{
		return g_unspentPoints;
	}

	void SetUnspentPoints(std::int32_t value)
	{
		g_unspentPoints = std::max(0, value);
	}

	std::int32_t GetERLevel()
	{
		return g_erLevel;
	}

	void SetERLevel(std::int32_t value)
	{
		g_erLevel = std::max(1, value);
	}
}

