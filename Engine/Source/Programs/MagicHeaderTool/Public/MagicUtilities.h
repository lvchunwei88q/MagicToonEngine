#pragma once

// Core Libraries
#include <Common/Checker.h>
#include <Tools/Singleton.h>

#include <iostream>
#include <chrono>

#include <string>
#include <vector>

#include "MagicGlobalContext.h"
#include "MagicConfig.h"

DISABLE_DLL_WARNINGS_PUSH;

namespace Utilitie {
	class GlobalTimer : public Singleton<GlobalTimer>
	{
	private:
		std::chrono::steady_clock::time_point startTime;

	public:
		GlobalTimer() {}
		~GlobalTimer() {}

		// Start the timer
		void start() {
			startTime = std::chrono::steady_clock::now();
		}

		// Return time taken
		long long elapsed_ms() const {
			auto endTime = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		}
	};

	using GTimer = decltype(GlobalTimer::Get());

	class Log {
	public:
		static void Info(const std::string& msg) {
			MGC ctx = MagicGlobalContext::Get();
#ifdef _DEBUG
			std::cout << "[" + std::to_string(ctx.GetInfo().CurrentTaskIndex) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][INFO] " << msg << "\n";
#endif
		}

		static void Warning(const std::string& msg) {
			MGC ctx = MagicGlobalContext::Get();
			std::cout << "[" + std::to_string(ctx.GetInfo().CurrentTaskIndex) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][WARNING] " << msg << "\n";
		}

		static void Error(const std::string& msg) {
			MGC ctx = MagicGlobalContext::Get();
			std::cout << "[" + std::to_string(ctx.GetInfo().CurrentTaskIndex) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][ERROR] " << msg << "\n";
		}
	};

	// Get file modification time
	size_t GetFileModificationTime(const std::wstring& filepath);
	// Read files in parallel
	std::vector<std::string> ReadFilesParallel(const std::vector<std::wstring>& filePaths);
}

DISABLE_DLL_WARNINGS_POP;