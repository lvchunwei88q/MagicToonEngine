#pragma once

// Core Libraries
#include <Common/Checker.h>
#include <Utilities/Singleton.h>

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
			std::cout << "[" + std::to_string(ctx.GetCurrentShowTaskIndex()) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][INFO] " << msg << "\n";
#endif
		}

		static void Warning(const std::string& msg) {
			MGC ctx = MagicGlobalContext::Get();
			std::cout << "[" + std::to_string(ctx.GetCurrentShowTaskIndex()) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][WARNING] " << msg << "\n";
		}

		static void Error(const std::string& msg) {
			MGC ctx = MagicGlobalContext::Get();
			std::cout << "[" + std::to_string(ctx.GetCurrentShowTaskIndex()) + "/" + std::to_string(ctx.GetInfo().TaskNum) + "][ERROR] " << msg << "\n";
		}
	};

	/**
	* @brief Generate a hash (using FNV-1a).
	* @return The hash value.
	*/
	uint64_t HashFNV1a64(const std::string_view s);

	/**
	 * @brief 64-bit content hash of a file (FNV-1a over the raw bytes).
	 * @details Used to decide whether reflection code has to be regenerated:
	 *          touching a file no longer counts as a change, only editing it does.
	 * @return The hash, or 0 when the file does not exist or cannot be read.
	 */
	uint64_t GetFileContentHash(const IO::path& filepath);

	/**
	 * @brief Content hash of many files, computed in parallel.
	 * @details `hashes` is resized to `filePaths.size()`; unreadable files get 0.
	 */
	void GetFileContentHashesParallel(const std::vector<IO::path>& filePaths, std::vector<uint64_t>& hashes);

	// Read files in parallel, returning false if any file could not be read.
	bool ReadFilesParallel(const std::vector<IO::path>& filePaths, std::vector<std::string>& contents);
}

DISABLE_DLL_WARNINGS_POP;
