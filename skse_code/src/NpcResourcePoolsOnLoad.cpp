#include "NpcResourcePoolsOnLoad.h"

#include "Attributes.h"
#include "Config.h"
#include "DerivedStats.h"
#include "pch.h"

namespace ER
{
	namespace
	{
		class NpcDerivedPoolsSink final : public RE::BSTEventSink<RE::TESObjectLoadedEvent>
		{
		public:
			static NpcDerivedPoolsSink* GetSingleton()
			{
				static NpcDerivedPoolsSink singleton;
				return std::addressof(singleton);
			}

			RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override
			{
				if (!a_event || !a_event->loaded) {
					return RE::BSEventNotifyControl::kContinue;
				}
				if (!ER::Config::ApplyNpcDerivedPoolsOnLoad()) {
					return RE::BSEventNotifyControl::kContinue;
				}

				const auto formID = a_event->formID;
				auto* task = SKSE::GetTaskInterface();
				if (!task) {
					return RE::BSEventNotifyControl::kContinue;
				}

				task->AddTask([formID]() {
					auto* form = RE::TESForm::LookupByID(formID);
					auto* refr = form ? form->AsReference() : nullptr;
					auto* actor = refr ? refr->As<RE::Actor>() : nullptr;
					if (!actor || actor->IsPlayerRef() || actor->IsDead()) {
						return;
					}
					if (!IsNPCAttributeEligible(actor)) {
						return;
					}
					ApplyDerivedResourcePoolsToActor(actor);
				});

				return RE::BSEventNotifyControl::kContinue;
			}
		};
	}

	void InstallNpcDerivedPoolsOnLoad()
	{
		static bool s_installed = false;
		if (s_installed) {
			return;
		}
		auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!holder) {
			logger::warn("NPC derived pools: ScriptEventSourceHolder unavailable");
			return;
		}
		holder->AddEventSink<RE::TESObjectLoadedEvent>(NpcDerivedPoolsSink::GetSingleton());
		s_installed = true;
		logger::info("Installed NPC derived pool apply (TESObjectLoaded)");
	}
}
