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

	std::optional<ER::Attribute> ParseAttr(const char* args)
	{
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
	}

	void AddPending(ER::Attribute a, std::int32_t delta)
	{
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
	}

	std::int32_t GetPending(ER::Attribute a)
	{
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
	}

	std::int32_t GetPendingPointCount()
	{
		return ER::PointsSpent(g_pendingDelta);
	}

	std::int32_t GetPendingGoldCost(std::int32_t baseLevel, std::int32_t pendingPoints)
	{
		baseLevel = std::max(1, baseLevel);
		pendingPoints = std::max(0, pendingPoints);

		std::int64_t total = 0;
		for (std::int32_t i = 0; i < pendingPoints; ++i) {
			total += static_cast<std::int64_t>(ER::GoldCostToLevelUp(baseLevel + i));
		}
		return static_cast<std::int32_t>(std::min<std::int64_t>(total, std::numeric_limits<std::int32_t>::max()));
	}

	void RecomputeAndApplyCurrentStats()
	{
		const auto snapshot = ER::GetCurrentStatsSnapshot();
		ER::ApplyToPlayer(snapshot.derived, snapshot.attrs, snapshot.erLevel);
	}


	void OnRequestInitState(const char*)
	{
		RecomputeAndApplyCurrentStats();
		Prisma::SendUpdateToUI();
	}

	void OnLevelUp(const char*)
	{
		const auto currentLevel = Persist::GetERLevel();
		const auto cost = ER::GoldCostToLevelUp(currentLevel);
		if (!ER::TrySpendPlayerGold(cost)) {
			Prisma::SendUpdateToUI();
			return;
		}
		Persist::SetERLevel(currentLevel + 1);
		Persist::SetUnspentPoints(Persist::GetUnspentPoints() + 1);
		Prisma::SendUpdateToUI();
	}

	void OnAllocatePoint(const char* args)
	{
		const auto optAttr = ParseAttr(args);
		if (!optAttr) {
			return;
		}

		const auto level = Persist::GetERLevel();
		const auto pendingBefore = GetPendingPointCount();
		const auto pendingAfter = pendingBefore + 1;
		const auto totalPendingCost = GetPendingGoldCost(level, pendingAfter);
		const auto gold = ER::GetPlayerGold();
		if (gold < totalPendingCost) {
			logger::info(
				"Allocate blocked: level={}, pending_before={}, pending_after={}, gold={}, total_pending_cost={}",
				level,
				pendingBefore,
				pendingAfter,
				gold,
				totalPendingCost);
			Prisma::SendUpdateToUI();
			return;
		}

		AddPending(*optAttr, 1);
		logger::info("Allocate ok: pending_points={} total_pending_cost={}", GetPendingPointCount(), totalPendingCost);
		Prisma::SendUpdateToUI();
	}

	void OnRefundPoint(const char* args)
	{
		const auto optAttr = ParseAttr(args);
		if (!optAttr) {
			return;
		}

		const auto existing = GetPending(*optAttr);
		if (existing <= 0) {
			return;
		}

		AddPending(*optAttr, -1);
		logger::info("Refund ok: pending_points={}", GetPendingPointCount());
		Prisma::SendUpdateToUI();
	}

	void OnConfirmAllocation(const char*)
	{
		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		const auto pendingPoints = GetPendingPointCount();
		if (pendingPoints <= 0) {
			logger::info("Confirm ignored: pending_points=0");
			return;
		}

		const auto levelBefore = Persist::GetERLevel();
		const auto totalPendingCost = GetPendingGoldCost(levelBefore, pendingPoints);
		const auto goldBefore = ER::GetPlayerGold();
		logger::info(
			"Confirm requested: level_before={}, pending_points={}, gold_before={}, total_pending_cost={}",
			levelBefore,
			pendingPoints,
			goldBefore,
			totalPendingCost);
		if (!ER::TrySpendPlayerGold(totalPendingCost)) {
			logger::warn(
				"Confirm failed spending gold: level_before={}, pending_points={}, gold_before={}, total_pending_cost={}",
				levelBefore,
				pendingPoints,
				goldBefore,
				totalPendingCost);
			Prisma::SendUpdateToUI();
			return;
		}

		auto committedAttrs = ER::GetAll(player);

		auto setAttr = [&](ER::Attribute a, std::int32_t delta) {
			if (delta <= 0) return;
			std::int32_t target = 0;
			switch (a) {
			case ER::Attribute::VIG: committedAttrs.vig += delta; target = committedAttrs.vig; break;
			case ER::Attribute::MND: committedAttrs.mnd += delta; target = committedAttrs.mnd; break;
			case ER::Attribute::END: committedAttrs.end += delta; target = committedAttrs.end; break;
			case ER::Attribute::STR: committedAttrs.str += delta; target = committedAttrs.str; break;
			case ER::Attribute::DEX: committedAttrs.dex += delta; target = committedAttrs.dex; break;
			case ER::Attribute::INT: committedAttrs.intl += delta; target = committedAttrs.intl; break;
			case ER::Attribute::FTH: committedAttrs.fth += delta; target = committedAttrs.fth; break;
			case ER::Attribute::ARC: committedAttrs.arc += delta; target = committedAttrs.arc; break;
			}
			ER::Set(player, a, target);
		};

		setAttr(ER::Attribute::VIG, g_pendingDelta.vig);
		setAttr(ER::Attribute::MND, g_pendingDelta.mnd);
		setAttr(ER::Attribute::END, g_pendingDelta.end);
		setAttr(ER::Attribute::STR, g_pendingDelta.str);
		setAttr(ER::Attribute::DEX, g_pendingDelta.dex);
		setAttr(ER::Attribute::INT, g_pendingDelta.intl);
		setAttr(ER::Attribute::FTH, g_pendingDelta.fth);
		setAttr(ER::Attribute::ARC, g_pendingDelta.arc);

		const auto levelAfter = levelBefore + pendingPoints;
		Persist::SetERLevel(levelAfter);
		ER::ApplyToPlayer(ER::ComputeDerived(committedAttrs), committedAttrs, levelAfter);
		const auto readbackAttrs = ER::GetAll(player);
		g_pendingDelta = {};
		logger::info(
			"Confirm applied: level_after={} stored_level={} vanilla_level={} attrs_target(vig,mnd,end,str,dex,int,fth,arc)=({},{},{},{},{},{},{},{}) attrs_readback=({},{},{},{},{},{},{},{}) pending_cleared=1",
			levelAfter,
			Persist::GetERLevel(),
			player->GetLevel(),
			committedAttrs.vig,
			committedAttrs.mnd,
			committedAttrs.end,
			committedAttrs.str,
			committedAttrs.dex,
			committedAttrs.intl,
			committedAttrs.fth,
			committedAttrs.arc,
			readbackAttrs.vig,
			readbackAttrs.mnd,
			readbackAttrs.end,
			readbackAttrs.str,
			readbackAttrs.dex,
			readbackAttrs.intl,
			readbackAttrs.fth,
			readbackAttrs.arc);
		Prisma::SendUpdateToUI();
	}

	void OnCancelAllocation(const char*)
	{
		g_pendingDelta = {};
		Prisma::Hide();
		auto msgQueue = RE::UIMessageQueue::GetSingleton();
		if (msgQueue) {
			msgQueue->AddMessage(RE::StatsMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			msgQueue->AddMessage(RE::LevelUpMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}

	void OnHideWindow(const char*)
	{
		Prisma::Hide();
		auto msgQueue = RE::UIMessageQueue::GetSingleton();
		if (msgQueue) {
			msgQueue->AddMessage(RE::LevelUpMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}

	void OnBack(const char*)
	{
		Prisma::Hide();
	}

	constexpr const char* kViewPath = "EldenRimLevelingSystem/index.html";

	void RegisterListeners(PrismaView view)
	{
		g_prismaUI->RegisterJSListener(view, "requestInitState", &OnRequestInitState);
		g_prismaUI->RegisterJSListener(view, "levelUp", &OnLevelUp);
		g_prismaUI->RegisterJSListener(view, "allocatePoint", &OnAllocatePoint);
		g_prismaUI->RegisterJSListener(view, "refundPoint", &OnRefundPoint);
		g_prismaUI->RegisterJSListener(view, "confirmAllocation", &OnConfirmAllocation);
		g_prismaUI->RegisterJSListener(view, "cancelAllocation", &OnCancelAllocation);
		g_prismaUI->RegisterJSListener(view, "hideWindow", &OnHideWindow);
		g_prismaUI->RegisterJSListener(view, "back", &OnBack);
	}

	std::string BuildStateJSON()
	{
		const auto current = ER::GetCurrentStatsSnapshot();
		const auto attrs = current.attrs;
		const auto spent = ER::PointsSpent(attrs);
		const auto level = current.erLevel;
		const auto pendingPoints = GetPendingPointCount();
		const auto levelUpCost = ER::GoldCostToLevelUp(level);
		const auto nextLevelUpCost = ER::GoldCostToLevelUp(level + pendingPoints);
		const auto pendingCost = GetPendingGoldCost(level, pendingPoints);
		const auto gold = ER::GetPlayerGold();
		const bool canLevelUp = gold >= levelUpCost;
		const bool canAllocate = gold >= GetPendingGoldCost(level, pendingPoints + 1);
		const bool canConfirm = pendingPoints > 0 && gold >= pendingCost;

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

		const auto pendingSnapshot = ER::BuildStatsSnapshot(pendingAttrs, level);

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
		j["points"] = { { "spent", spent }, { "level", level }, { "pending", pendingPoints } };
		j["gold"] = {
			{ "current", gold },
			{ "levelUpCost", levelUpCost },
			{ "nextLevelUpCost", nextLevelUpCost },
			{ "pendingCost", pendingCost },
			{ "canLevelUp", canLevelUp },
			{ "canAllocate", canAllocate },
			{ "canConfirm", canConfirm }
		};
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
			{ "maxHP", current.derived.maxHP },
			{ "maxMP", current.derived.maxMP },
			{ "maxSP", current.derived.maxSP },
			{ "carryWeight", current.derived.carryWeight },
			{ "defense",
				{
					{ "physical", current.sheet.l1Phys },
					{ "magic", current.sheet.l1Magic },
					{ "fire", current.sheet.l1Fire },
					{ "lightning", current.sheet.l1Lightning },
					{ "frost", current.sheet.l1Frost },
					{ "poison", current.sheet.l1Poison },
				} },
			{ "thresholds",
				{
					{ "immunity", current.sheet.thImmunity },
					{ "robustness", current.sheet.thRobustness },
					{ "focus", current.sheet.thFocus },
					{ "vitality", current.sheet.thVitality },
					{ "madness", current.sheet.thMadness },
				} },
			{ "equipLoad",
				{
					{ "max", current.sheet.equipLoadMax },
					{ "light", current.sheet.equipLoadLight },
					{ "medium", current.sheet.equipLoadMedium },
					{ "heavy", current.sheet.equipLoadHeavy },
				} },
		};
		j["derivedPending"] = {
			{ "maxHP", pendingSnapshot.derived.maxHP },
			{ "maxMP", pendingSnapshot.derived.maxMP },
			{ "maxSP", pendingSnapshot.derived.maxSP },
			{ "carryWeight", pendingSnapshot.derived.carryWeight },
			{ "defense",
				{
					{ "physical", pendingSnapshot.sheet.l1Phys },
					{ "magic", pendingSnapshot.sheet.l1Magic },
					{ "fire", pendingSnapshot.sheet.l1Fire },
					{ "lightning", pendingSnapshot.sheet.l1Lightning },
					{ "frost", pendingSnapshot.sheet.l1Frost },
					{ "poison", pendingSnapshot.sheet.l1Poison },
				} },
			{ "thresholds",
				{
					{ "immunity", pendingSnapshot.sheet.thImmunity },
					{ "robustness", pendingSnapshot.sheet.thRobustness },
					{ "focus", pendingSnapshot.sheet.thFocus },
					{ "vitality", pendingSnapshot.sheet.thVitality },
					{ "madness", pendingSnapshot.sheet.thMadness },
				} },
			{ "equipLoad",
				{
					{ "max", pendingSnapshot.sheet.equipLoadMax },
					{ "light", pendingSnapshot.sheet.equipLoadLight },
					{ "medium", pendingSnapshot.sheet.equipLoadMedium },
					{ "heavy", pendingSnapshot.sheet.equipLoadHeavy },
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
		logger::info("ShowLevelUp called (createdView={}, view={})", g_createdView, g_view);

		if (!g_createdView) {
			g_createdView = true;
			logger::info("Creating PrismaUI view: {}", kViewPath);
			g_view = g_prismaUI->CreateView(kViewPath, [](PrismaView currentView) {
				logger::info("PrismaUI view DOM ready (view={})", currentView);
				RegisterListeners(currentView);
				g_prismaUI->Show(currentView);
				RecomputeAndApplyCurrentStats();
				Prisma::SendUpdateToUI();
				g_prismaUI->Focus(currentView, true);
			});
			logger::info("CreateView returned view={}", g_view);
			return;
		}

		g_prismaUI->Show(g_view);
		RecomputeAndApplyCurrentStats();
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

