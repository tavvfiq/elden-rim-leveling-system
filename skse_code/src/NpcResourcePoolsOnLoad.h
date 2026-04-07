#pragma once

namespace ER
{
	// Registers TESObjectLoadedEvent sink: ActorTypeNPC actors get ER-derived base pools once when loaded.
	void InstallNpcDerivedPoolsOnLoad();
}
