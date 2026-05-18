#pragma once

#include "Common/LOG_API.h"

#include <sstream>
#include <string>
#include <format>
#include <deque>  // 环形缓冲区

#define IN_SPECIAL_NAMESPACE

#ifdef IN_SPECIAL_NAMESPACE 
#define LOGSPACE LOG
#else
#define LOGSPACE
#endif

// 宏定义（方便使用）
#define LOG_DEBUG(...)   LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Debug,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_INFO(...)    LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Info,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_WARNING(...) LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Warning,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_ERROR(...)   LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Error,__FILE__,__LINE__,__VA_ARGS__) 

namespace LOG {
	// 日志级别枚举
	enum class LogLevel {
		Debug,
		Info,
		Warning,
		Error
	};

	struct LogEntry {
		LogLevel    level;
		std::string timestamp;
		std::string file;
		int         line;
		std::string message;
	};

	class LOG_API LogInterface
	{
		template <typename... Args>
		std::string FormatString(Args &&...args) {
			std::ostringstream oss;
			(oss << ... << std::forward<Args>(args));
			return oss.str();
		}
	public:
		LogInterface() = default;
		// 获取流对象
		template <typename... Args>
		void Log(LogLevel level, const char* file, int line, Args &&...args) {
			LogImpl(level, file, line, FormatString(args...));
		}

		virtual void SwapBuffers() = 0; // 交换缓冲区
		virtual void ClearRecentEntries() = 0; // 清空缓冲区
		virtual const std::deque<LogEntry>& GetEntries() const = 0;
		// 获取Level
		virtual std::string GetLevelString(LogLevel level) const = 0;

	private:
		virtual void LogImpl(LogLevel level, const char* file, int line, const std::string& message) = 0;
	};

	LOG_API LogInterface* GetLogInstance();
}

#undef IN_SPECIAL_NAMESPACE