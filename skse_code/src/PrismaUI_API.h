/*
 * PrismaUI modder API header.
 * Copied/adapted from your local PrismaUI template usage (New-Skill-Menu).
 */
#pragma once

#include <cstdint>

typedef std::uint64_t PrismaView;

namespace PRISMA_UI_API
{
	constexpr const auto PrismaUIPluginName = "PrismaUI";

	enum class InterfaceVersion : std::uint8_t
	{
		V1
	};

	using OnDomReadyCallback = void (*)(PrismaView view);
	using JSCallback = void (*)(const char* result);
	using JSListenerCallback = void (*)(const char* argument);

	class IVPrismaUI1
	{
	public:
		virtual PrismaView CreateView(const char* htmlPath, OnDomReadyCallback onDomReadyCallback = nullptr) noexcept = 0;
		virtual void Invoke(PrismaView view, const char* script, JSCallback callback = nullptr) noexcept = 0;
		virtual void InteropCall(PrismaView view, const char* functionName, const char* argument) noexcept = 0;
		virtual void RegisterJSListener(PrismaView view, const char* functionName, JSListenerCallback callback) noexcept = 0;

		virtual bool HasFocus(PrismaView view) noexcept = 0;
		virtual bool Focus(PrismaView view, bool pauseGame = false, bool disableFocusMenu = false) noexcept = 0;
		virtual void Unfocus(PrismaView view) noexcept = 0;

		virtual void Show(PrismaView view) noexcept = 0;
		virtual void Hide(PrismaView view) noexcept = 0;
		virtual bool IsHidden(PrismaView view) noexcept = 0;

		virtual int GetScrollingPixelSize(PrismaView view) noexcept = 0;
		virtual void SetScrollingPixelSize(PrismaView view, int pixelSize) noexcept = 0;

		virtual bool IsValid(PrismaView view) noexcept = 0;
		virtual void Destroy(PrismaView view) noexcept = 0;

		virtual void SetOrder(PrismaView view, int order) noexcept = 0;
		virtual int GetOrder(PrismaView view) noexcept = 0;

		virtual void CreateInspectorView(PrismaView view) noexcept = 0;
		virtual void SetInspectorVisibility(PrismaView view, bool visible) noexcept = 0;
		virtual bool IsInspectorVisible(PrismaView view) noexcept = 0;
		virtual void SetInspectorBounds(PrismaView view, float topLeftX, float topLeftY, unsigned int width, unsigned int height) noexcept = 0;

		virtual bool HasAnyActiveFocus() noexcept = 0;
	};

	using RequestPluginAPIFn = void* (*)(InterfaceVersion interfaceVersion);

	[[nodiscard]] inline void* RequestPluginAPI(const InterfaceVersion interfaceVersion = InterfaceVersion::V1)
	{
		auto pluginHandle = GetModuleHandleW(L"PrismaUI.dll");
		if (!pluginHandle) {
			return nullptr;
		}

		auto requestAPIFunction = reinterpret_cast<RequestPluginAPIFn>(GetProcAddress(pluginHandle, "RequestPluginAPI"));
		if (!requestAPIFunction) {
			return nullptr;
		}

		return requestAPIFunction(interfaceVersion);
	}
}

