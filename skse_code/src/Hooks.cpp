#include "Hooks.h"

#include "Prisma.h"
#include "pch.h"

namespace
{
	RE::InputEvent* FilterInputEvents(RE::InputEvent* head)
	{
		if (!head) {
			return nullptr;
		}

		// Only block if our Prisma view is active and specifically during level-up allocation.
		if (Prisma::IsHidden() || !Prisma::IsLevelUpMenuOpen()) {
			return head;
		}

		auto* userEvents = RE::UserEvents::GetSingleton();
		if (!userEvents) {
			return head;
		}

		RE::InputEvent* prev = nullptr;
		RE::InputEvent* curr = head;
		RE::InputEvent* newHead = head;

		while (curr) {
			bool blocked = false;

			if (curr->device == RE::INPUT_DEVICE::kKeyboard) {
				if (auto* button = curr->AsButtonEvent()) {
					if (button->IsDown()) {
						auto action = button->QUserEvent();
						if (action == userEvents->cancel || action == userEvents->tweenMenu) {
							blocked = true;
						}
					}
				}
			}

			if (blocked) {
				if (prev) {
					prev->next = curr->next;
				} else {
					newHead = curr->next;
				}
				curr = curr->next;
				continue;
			}

			prev = curr;
			curr = curr->next;
		}

		return newHead;
	}

	struct ProcessInputQueueHook
	{
		static void thunk(RE::BSTEventSource<RE::InputEvent*>* dispatcher, RE::InputEvent* const* events)
		{
			auto* filtered = FilterInputEvents(const_cast<RE::InputEvent*>(*events));
			RE::InputEvent* const filteredPtr = filtered;
			original(dispatcher, &filteredPtr);
		}

		static void Install()
		{
			auto& trampoline = SKSE::GetTrampoline();
			original = trampoline.write_call<5>(
				REL::RelocationID(67315, 68617).address() + REL::Relocate(0x7B, 0x7B),
				thunk);
			logger::info("Installed input queue hook");
		}

		static inline REL::Relocation<decltype(thunk)> original;
	};

	class LevelUpMenuHook
	{
	public:
		static RE::UI_MESSAGE_RESULTS ProcessMessage_Hook(RE::LevelUpMenu* a_this, RE::UIMessage& a_message)
		{
			auto result = _ProcessMessage(a_this, a_message);

			if (a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
				if (a_this && a_this->uiMovie) {
					a_this->uiMovie->SetVisible(false);
					RE::GFxValue alpha(0.0);
					a_this->uiMovie->SetVariable("_root._alpha", &alpha);
				}

				Prisma::SetLevelUpMenuOpen(true);
				Prisma::ShowLevelUp();
				Prisma::SendUpdateToUI();
			} else if (a_message.type == RE::UI_MESSAGE_TYPE::kHide) {
				Prisma::SetLevelUpMenuOpen(false);
				Prisma::Hide();
				return RE::UI_MESSAGE_RESULTS::kHandled;
			}

			return result;
		}

		static void Install()
		{
			REL::Relocation<std::uintptr_t> vtable(RE::VTABLE_LevelUpMenu[0]);
			_ProcessMessage = vtable.write_vfunc(0x4, ProcessMessage_Hook);
			logger::info("Installed LevelUpMenu ProcessMessage hook");
		}

	private:
		static inline REL::Relocation<decltype(ProcessMessage_Hook)> _ProcessMessage;
	};
}

namespace Hooks
{
	void Install()
	{
		SKSE::AllocTrampoline(64);
		ProcessInputQueueHook::Install();
		LevelUpMenuHook::Install();
	}
}

