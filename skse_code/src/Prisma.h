#pragma once

namespace Prisma
{
	// Must be called on/after SKSE kPostLoad.
	void Install();

	// Level-up allocation view controls.
	void ShowLevelUp();
	void Hide();

	// State helpers.
	bool IsReady();
	bool IsHidden();

	void SetLevelUpMenuOpen(bool open);
	bool IsLevelUpMenuOpen();

	// UI messaging.
	void SendUpdateToUI();
	void TriggerBack();
}

