#pragma once

namespace Prisma
{
	// Must be called on/after SKSE kPostLoad.
	void Install();

	// Level-up allocation view controls.
	void ShowLevelUp();
	void Hide();

	// Sleep / menu gating (see [Sleep] in eras.ini).
	void NotifySleepStarted();
	void NotifySleepEnded();

	// State helpers.
	bool IsReady();
	bool IsHidden();

	void SetLevelUpMenuOpen(bool open);
	bool IsLevelUpMenuOpen();

	// UI messaging.
	void SendUpdateToUI();
	void TriggerBack();
}

