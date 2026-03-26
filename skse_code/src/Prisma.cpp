#include "Prisma.h"

#include "Attributes.h"
#include "DerivedStats.h"
#include "Economy.h"
#include "PrismaUI_API.h"
#include "Serialization.h"
#include "pch.h"

using json = nlohmann::json;

namespace
{
	PRISMA_UI_API::IVPrismaUI1* g_prismaUI{ nullptr };
	PrismaView g_view{ 0 };
	bool g_createdView{ false };
	bool g_levelUpMenuOpen{ false };
	ER::AttributeSet g_pendingDelta{};

	constexpr const char* kViewPath =
#ifdef DEV_SERVER
		"http://localhost:5173";
#else
		"AspectsAttributes/index.html";
#endif

	void RegisterListeners(PrismaView view)
	{
		auto parseAttr = [](const char* args) -> std::optional<ER::Attribute> {
			if (!args) {
				return std::nullopt;
			}
			try {
				const auto j = json::parse(args);
				const auto key = j.value("attr", std::string{});
				if (key == "vig") return ER::Attribute::VIG;
				if (key == "mnd") return ER::Attribute::MND;
				if (key == "end") return ER::Attribute::END;
				if (key == "str") return ER::Attribute::STR;
				if (key == "dex") return ER::Attribute::DEX;
				if (key == "int") return ER::Attribute::INT;
				if (key == "fth") return ER::Attribute::FTH;
				if (key == "arc") return ER::Attribute::ARC;
			} catch (...) {
			}
			return std::nullopt;
		};

		auto addPending = [](ER::Attribute a, std::int32_t delta) {
			switch (a) {
			case ER::Attribute::VIG: g_pendingDelta.vig += delta; break;
			case ER::Attribute::MND: g_pendingDelta.mnd += delta; break;
			case ER::Attribute::END: g_pendingDelta.end += delta; break;
			case ER::Attribute::STR: g_pendingDelta.str += delta; break;
			case ER::Attribute::DEX: g_pendingDelta.dex += delta; break;
			case ER::Attribute::INT: g_pendingDelta.intl += delta; break;
			case ER::Attribute::FTH: g_pendingDelta.fth += delta; break;
			case ER::Attribute::ARC: g_pendingDelta.arc += delta; break;
			}
		};

		auto getPending = [](ER::Attribute a) -> std::int32_t {
			switch (a) {
			case ER::Attribute::VIG: return g_pendingDelta.vig;
			case ER::Attribute::MND: return g_pendingDelta.mnd;
			case ER::Attribute::END: return g_pendingDelta.end;
			case ER::Attribute::STR: return g_pendingDelta.str;
			case ER::Attribute::DEX: return g_pendingDelta.dex;
			case ER::Attribute::INT: return g_pendingDelta.intl;
			case ER::Attribute::FTH: return g_pendingDelta.fth;
			case ER::Attribute::ARC: return g_pendingDelta.arc;
			}
			return 0;
		};

		g_prismaUI->RegisterJSListener(view, "requestInitState", [](const char*) {
			Prisma::SendUpdateToUI();
		});

		g_prismaUI->RegisterJSListener(view, "levelUp", [](const char*) {
			const auto currentLevel = Persist::GetERLevel();
			const auto cost = ER::GoldCostToLevelUp(currentLevel);
			if (!ER::TrySpendPlayerGold(cost)) {
				Prisma::SendUpdateToUI();
				return;
			}
			Persist::SetERLevel(currentLevel + 1);
			Persist::SetUnspentPoints(Persist::GetUnspentPoints() + 1);
			Prisma::SendUpdateToUI();
		});

		g_prismaUI->RegisterJSListener(view, "allocatePoint", [parseAttr, addPending](const char* args) {
			const auto optAttr = parseAttr(args);
			if (!optAttr) {
				return;
			}

			const auto unspent = Persist::GetUnspentPoints();
			if (unspent <= 0) {
				return;
			}

			addPending(*optAttr, 1);
			Persist::SetUnspentPoints(unspent - 1);
			Prisma::SendUpdateToUI();
		});

		g_prismaUI->RegisterJSListener(view, "refundPoint", [parseAttr, addPending, getPending](const char* args) {
			const auto optAttr = parseAttr(args);
			if (!optAttr) {
				return;
			}

			const auto existing = getPending(*optAttr);
			if (existing <= 0) {
				return;
			}

			addPending(*optAttr, -1);
			Persist::SetUnspentPoints(Persist::GetUnspentPoints() + 1);
			Prisma::SendUpdateToUI();
		});

		g_prismaUI->RegisterJSListener(view, "confirmAllocation", [](const char*) {
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!player) {
				return;
			}

			if (ER::PointsSpent(g_pendingDelta) <= 0) {
				return;
			}

			// Commit: write AVs.
			auto setAttr = [&](ER::Attribute a, std::int32_t delta) {
				if (delta <= 0) return;
				const auto current = ER::Get(player, a);
				ER::Set(player, a, current + delta);
			};

			setAttr(ER::Attribute::VIG, g_pendingDelta.vig);
			setAttr(ER::Attribute::MND, g_pendingDelta.mnd);
			setAttr(ER::Attribute::END, g_pendingDelta.end);
			setAttr(ER::Attribute::STR, g_pendingDelta.str);
			setAttr(ER::Attribute::DEX, g_pendingDelta.dex);
			setAttr(ER::Attribute::INT, g_pendingDelta.intl);
			setAttr(ER::Attribute::FTH, g_pendingDelta.fth);
			setAttr(ER::Attribute::ARC, g_pendingDelta.arc);

			// Recompute derived stats and apply.
			ER::ApplyToPlayer(ER::ComputeDerived(ER::GetAll(player)));

			// Clear pending.
			g_pendingDelta = {};

			Prisma::SendUpdateToUI();
		});

		g_prismaUI->RegisterJSListener(view, "cancelAllocation", [](const char*) {
			// Refund unspent points for all pending allocations.
			Persist::SetUnspentPoints(Persist::GetUnspentPoints() + ER::PointsSpent(g_pendingDelta));
			g_pendingDelta = {};
			Prisma::Hide();
		});

		g_prismaUI->RegisterJSListener(view, "hideWindow", [](const char*) {
			Prisma::Hide();
			auto msgQueue = RE::UIMessageQueue::GetSingleton();
			if (msgQueue) {
				msgQueue->AddMessage(RE::LevelUpMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			}
		});

		g_prismaUI->RegisterJSListener(view, "back", [](const char*) {
			Prisma::Hide();
		});
	}

	std::string BuildStateJSON()
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		const auto attrs = ER::GetAll(player);
		const auto spent = ER::PointsSpent(attrs);
		const auto level = Persist::GetERLevel();
		const auto unspent = Persist::GetUnspentPoints();
		const auto levelUpCost = ER::GoldCostToLevelUp(level);
		const auto gold = ER::GetPlayerGold();
		const bool canLevelUp = gold >= levelUpCost;

		// For arrow displays we compute a “pending” derived set.
		auto pendingAttrs = attrs;
		pendingAttrs.vig += g_pendingDelta.vig;
		pendingAttrs.mnd += g_pendingDelta.mnd;
		pendingAttrs.end += g_pendingDelta.end;
		pendingAttrs.str += g_pendingDelta.str;
		pendingAttrs.dex += g_pendingDelta.dex;
		pendingAttrs.intl += g_pendingDelta.intl;
		pendingAttrs.fth += g_pendingDelta.fth;
		pendingAttrs.arc += g_pendingDelta.arc;

		const auto derived = ER::ComputeDerived(attrs);
		const auto derivedPending = ER::ComputeDerived(pendingAttrs);

		const auto sheet = ER::ComputePublishedSheetAVGs(attrs, level, derived);
		const auto sheetPending = ER::ComputePublishedSheetAVGs(pendingAttrs, level, derivedPending);

		json j;
		j["ready"] = true;
		j["levelUpMenuOpen"] = g_levelUpMenuOpen;
		j["attributes"] = {
			{ "vig", attrs.vig },
			{ "mnd", attrs.mnd },
			{ "end", attrs.end },
			{ "str", attrs.str },
			{ "dex", attrs.dex },
			{ "int", attrs.intl },
			{ "fth", attrs.fth },
			{ "arc", attrs.arc },
		};
		j["points"] = { { "spent", spent }, { "level", level }, { "unspent", unspent } };
		j["gold"] = { { "current", gold }, { "levelUpCost", levelUpCost }, { "canLevelUp", canLevelUp } };
		j["pending"] = {
			{ "attributes",
				{
					{ "vig", g_pendingDelta.vig },
					{ "mnd", g_pendingDelta.mnd },
					{ "end", g_pendingDelta.end },
					{ "str", g_pendingDelta.str },
					{ "dex", g_pendingDelta.dex },
					{ "int", g_pendingDelta.intl },
					{ "fth", g_pendingDelta.fth },
					{ "arc", g_pendingDelta.arc },
				} },
		};
		j["derived"] = {
			{ "maxHP", derived.maxHP },
			{ "maxMP", derived.maxMP },
			{ "maxSP", derived.maxSP },
			{ "carryWeight", derived.carryWeight },
			{ "defense",
				{
					{ "physical", sheet.l1Phys },
					{ "magic", sheet.l1Magic },
					{ "fire", sheet.l1Fire },
					{ "lightning", sheet.l1Lightning },
					{ "frost", sheet.l1Frost },
					{ "poison", sheet.l1Poison },
				} },
			{ "thresholds",
				{
					{ "immunity", sheet.thImmunity },
					{ "robustness", sheet.thRobustness },
					{ "focus", sheet.thFocus },
					{ "vitality", sheet.thVitality },
					{ "madness", sheet.thMadness },
				} },
			{ "equipLoad",
				{
					{ "max", sheet.equipLoadMax },
					{ "light", sheet.equipLoadLight },
					{ "medium", sheet.equipLoadMedium },
					{ "heavy", sheet.equipLoadHeavy },
				} },
		};
		j["derivedPending"] = {
			{ "maxHP", derivedPending.maxHP },
			{ "maxMP", derivedPending.maxMP },
			{ "maxSP", derivedPending.maxSP },
			{ "carryWeight", derivedPending.carryWeight },
			{ "defense",
				{
					{ "physical", sheetPending.l1Phys },
					{ "magic", sheetPending.l1Magic },
					{ "fire", sheetPending.l1Fire },
					{ "lightning", sheetPending.l1Lightning },
					{ "frost", sheetPending.l1Frost },
					{ "poison", sheetPending.l1Poison },
				} },
			{ "thresholds",
				{
					{ "immunity", sheetPending.thImmunity },
					{ "robustness", sheetPending.thRobustness },
					{ "focus", sheetPending.thFocus },
					{ "vitality", sheetPending.thVitality },
					{ "madness", sheetPending.thMadness },
				} },
			{ "equipLoad",
				{
					{ "max", sheetPending.equipLoadMax },
					{ "light", sheetPending.equipLoadLight },
					{ "medium", sheetPending.equipLoadMedium },
					{ "heavy", sheetPending.equipLoadHeavy },
				} },
		};
		return j.dump();
	}
}

namespace Prisma
{
	void Install()
	{
		if (g_prismaUI) {
			return;
		}

		void* api = PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1);
		g_prismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(api);

		if (!g_prismaUI) {
			logger::error("PrismaUI API not available (PrismaUI.dll not loaded?)");
			return;
		}

		logger::info("PrismaUI API acquired");
	}

	bool IsReady()
	{
		return g_prismaUI != nullptr;
	}

	void ShowLevelUp()
	{
		if (!g_prismaUI) {
			logger::warn("ShowLevelUp called before Prisma::Install");
			return;
		}

		if (!g_createdView) {
			g_createdView = true;
			logger::info("Creating PrismaUI view: {}", kViewPath);
			g_view = g_prismaUI->CreateView(kViewPath, [](PrismaView currentView) {
				RegisterListeners(currentView);
				Prisma::SendUpdateToUI();
				g_prismaUI->Focus(currentView, true);
			});
			return;
		}

		g_prismaUI->Show(g_view);
		SendUpdateToUI();
		g_prismaUI->Focus(g_view, true);
	}

	void Hide()
	{
		if (!g_prismaUI || !g_createdView) {
			return;
		}

		g_prismaUI->Hide(g_view);
	}

	bool IsHidden()
	{
		if (!g_prismaUI || !g_createdView) {
			return true;
		}
		return g_prismaUI->IsHidden(g_view);
	}

	void SetLevelUpMenuOpen(bool open)
	{
		g_levelUpMenuOpen = open;
	}

	bool IsLevelUpMenuOpen()
	{
		return g_levelUpMenuOpen;
	}

	void SendUpdateToUI()
	{
		if (!g_prismaUI || !g_createdView) {
			return;
		}

		const std::string jsonStr = BuildStateJSON();
		std::string script = "window.dispatchEvent(new CustomEvent('updateState', { detail: " + jsonStr + " }));";
		g_prismaUI->Invoke(g_view, script.c_str());
	}

	void TriggerBack()
	{
		if (!g_prismaUI || !g_createdView) {
			return;
		}
		g_prismaUI->Invoke(g_view, "window.dispatchEvent(new CustomEvent('hardwareBack'));");
	}
}

