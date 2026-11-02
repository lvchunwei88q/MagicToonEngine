#pragma once

// Core Libraries
#include <Common/Checker.h>
#include <Tools/Singleton.h>

#include <iostream>
#include <chrono>

#include <string>
#include <vector>

namespace CF {
///////////////////////////// Config
#define DETAILED 1
constexpr int SingleJobCapacity = 100; // 数量

extern size_t MHT_JobNum;
extern size_t MHT_CurrentJob;
/////////////////////////////
}

DISABLE_DLL_WARNINGS_PUSH;

namespace TOOL {
	class Timer {
	private:
		std::chrono::steady_clock::time_point startTime;

	public:
		Timer() {
			start();
		}

		void start() {
			startTime = std::chrono::steady_clock::now();
		}

		// 返回经过的毫秒数
		long long elapsed_ms() const {
			auto endTime = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		}

		void reset() {
			start();
		}
	};

	class Log {
	public:
		static void Info(const std::string& msg) {
#ifdef _DEBUG
			std::cout << "[" + std::to_string(CF::MHT_CurrentJob) + "/" + std::to_string(CF::MHT_JobNum) + "][INFO] " << msg << "\n";
#endif
		}

		static void Warning(const std::string& msg) {
			std::cout << "[" + std::to_string(CF::MHT_CurrentJob) + "/" + std::to_string(CF::MHT_JobNum) + "][WARNING] " << msg << "\n";
		}

		static void Error(const std::string& msg) {
			std::cout << "[" + std::to_string(CF::MHT_CurrentJob) + "/" + std::to_string(CF::MHT_JobNum) + "][ERROR] " << msg << "\n";
		}
	};

	// 获取文件修改时间
	size_t GetFileModificationTime(const std::wstring& filepath);

	// 并行读取文件
	std::vector<std::string> ReadFilesParallel(const std::vector<std::wstring>& filePaths);
}

DISABLE_DLL_WARNINGS_POP;