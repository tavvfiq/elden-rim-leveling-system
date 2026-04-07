#include "SleepAllocationGate.h"

#include "pch.h"

// CommonLib forward-declares TESSleepStartEvent but does not ship a header; empty payload is enough for the sink.
namespace RE
{
	struct TESSleepStartEvent {};
}

#include "Config.h"
#include "Prisma.h"

namespace ER
{
	namespace
	{
		class SleepStartSink final : public RE::BSTEventSink<RE::TESSleepStartEvent>
		{
		public:
			static SleepStartSink* GetSingleton()
			{
				static SleepStartSink s;
				return std::addressof(s);
			}

			RE::BSEventNotifyControl ProcessEvent(const RE::TESSleepStartEvent*, RE::BSTEventSource<RE::TESSleepStartEvent>*) override
			{
				Prisma::NotifySleepStarted();
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		class SleepStopSink final : public RE::BSTEventSink<RE::TESSleepStopEvent>
		{
		public:
			static SleepStopSink* GetSingleton()
			{
				static SleepStopSink s;
				return std::addressof(s);
			}

			RE::BSEventNotifyControl ProcessEvent(const RE::TESSleepStopEvent*, RE::BSTEventSource<RE::TESSleepStopEvent>*) override
			{
				Prisma::NotifySleepEnded();
				return RE::BSEventNotifyControl::kContinue;
			}
		};
	}

	void InstallSleepAllocationGate()
	{
		static bool s_installed = false;
		if (s_installed) {
			return;
		}
		auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!holder) {
			logger::warn("Sleep allocation gate: ScriptEventSourceHolder unavailable");
			return;
		}
		holder->AddEventSink<RE::TESSleepStartEvent>(SleepStartSink::GetSingleton());
		// holder->AddEventSink<RE::TESSleepStopEvent>(SleepStopSink::GetSingleton());
		s_installed = true;
		logger::info("Installed sleep allocation / skill-menu hooks (TESSleepStart/Stop)");
	}
}
