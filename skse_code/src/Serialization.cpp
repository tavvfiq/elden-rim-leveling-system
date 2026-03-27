#include "pch.h"

#include "Serialization.h"

namespace
{
	constexpr std::uint32_t kUniqueID = 'ASPA';
	constexpr std::uint32_t kRecord_Unspent = 'UNSP';
	constexpr std::uint32_t kRecord_ERLevel = 'ERLV';
	constexpr std::uint32_t kRecord_Attrs = 'ATTR';

	std::int32_t g_unspentPoints = 0;
	std::int32_t g_erLevel = 1;
	ER::AttributeSet g_attrs{};

	void SkipRecordData(SKSE::SerializationInterface* intfc, std::uint32_t length)
	{
		if (!intfc || length == 0) {
			return;
		}

		std::byte scratch[256]{};
		std::uint32_t remaining = length;
		while (remaining > 0) {
			const auto chunk = std::min<std::uint32_t>(remaining, static_cast<std::uint32_t>(sizeof(scratch)));
			const auto read = intfc->ReadRecordData(scratch, chunk);
			if (read == 0) {
				break;
			}
			remaining -= read;
		}
	}

	void OnSave(SKSE::SerializationInterface* intfc)
	{
		if (!intfc) {
			return;
		}

		if (!intfc->OpenRecord(kRecord_Unspent, 1)) {
			return;
		}
		if (!intfc->WriteRecordData(&g_unspentPoints, sizeof(g_unspentPoints))) {
			return;
		}

		if (!intfc->OpenRecord(kRecord_ERLevel, 1)) {
			return;
		}
		if (!intfc->WriteRecordData(&g_erLevel, sizeof(g_erLevel))) {
			return;
		}

		if (!intfc->OpenRecord(kRecord_Attrs, 1)) {
			return;
		}
		if (!intfc->WriteRecordData(&g_attrs.vig, sizeof(g_attrs.vig))) return;
		if (!intfc->WriteRecordData(&g_attrs.mnd, sizeof(g_attrs.mnd))) return;
		if (!intfc->WriteRecordData(&g_attrs.end, sizeof(g_attrs.end))) return;
		if (!intfc->WriteRecordData(&g_attrs.str, sizeof(g_attrs.str))) return;
		if (!intfc->WriteRecordData(&g_attrs.dex, sizeof(g_attrs.dex))) return;
		if (!intfc->WriteRecordData(&g_attrs.intl, sizeof(g_attrs.intl))) return;
		if (!intfc->WriteRecordData(&g_attrs.fth, sizeof(g_attrs.fth))) return;
		if (!intfc->WriteRecordData(&g_attrs.arc, sizeof(g_attrs.arc))) return;
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
			case kRecord_Attrs: {
				ER::AttributeSet attrs{};
				if (intfc->ReadRecordData(&attrs.vig, sizeof(attrs.vig)) != sizeof(attrs.vig)) break;
				if (intfc->ReadRecordData(&attrs.mnd, sizeof(attrs.mnd)) != sizeof(attrs.mnd)) break;
				if (intfc->ReadRecordData(&attrs.end, sizeof(attrs.end)) != sizeof(attrs.end)) break;
				if (intfc->ReadRecordData(&attrs.str, sizeof(attrs.str)) != sizeof(attrs.str)) break;
				if (intfc->ReadRecordData(&attrs.dex, sizeof(attrs.dex)) != sizeof(attrs.dex)) break;
				if (intfc->ReadRecordData(&attrs.intl, sizeof(attrs.intl)) != sizeof(attrs.intl)) break;
				if (intfc->ReadRecordData(&attrs.fth, sizeof(attrs.fth)) != sizeof(attrs.fth)) break;
				if (intfc->ReadRecordData(&attrs.arc, sizeof(attrs.arc)) != sizeof(attrs.arc)) break;
				g_attrs = attrs;
				break;
			}
			default:
				// CommonLibSSE-NG doesn't expose SkipRecord(); read & discard.
				SkipRecordData(intfc, length);
				break;
			}
		}
	}

	void OnRevert(SKSE::SerializationInterface*)
	{
		g_unspentPoints = 0;
		g_erLevel = 1;
		g_attrs = {};
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

	ER::AttributeSet GetAttributes()
	{
		return g_attrs;
	}

	void SetAttributes(const ER::AttributeSet& value)
	{
		g_attrs = value;
	}
}

