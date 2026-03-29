#include "Log.h"

#include "pch.h"

namespace Log
{
	void Init()
	{
		auto logsFolder = SKSE::log::log_directory();
		if (!logsFolder) {
			SKSE::stl::report_and_fail("Unable to resolve SKSE logs directory.");
		}

		*logsFolder /= "ERAS.log";

		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logsFolder->string(), true);
		auto defaultLogger = std::make_shared<spdlog::logger>("global log", std::move(sink));

		spdlog::set_default_logger(std::move(defaultLogger));
		spdlog::set_level(spdlog::level::info);
		spdlog::flush_on(spdlog::level::info);
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
	}
}

