#pragma once

#include "Common/LOG_API.h"

#include <sstream>
#include <string>
#include <format>
#include <deque>  // Circular buffer

// Macro definition (for easy use)
#define LOG_DEBUG(...)   ::LOG::GetLogInstance()->Log(::LOG::LogLevel::Debug,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_INFO(...)    ::LOG::GetLogInstance()->Log(::LOG::LogLevel::Info,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_WARNING(...) ::LOG::GetLogInstance()->Log(::LOG::LogLevel::Warning,__FILE__,__LINE__,__VA_ARGS__) 
#define LOG_ERROR(...)   ::LOG::GetLogInstance()->Log(::LOG::LogLevel::Error,__FILE__,__LINE__,__VA_ARGS__) 

namespace LOG {
	// Log Level Enum
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
		// Get the stream object
		template <typename... Args>
		void Log(LogLevel level, const char* file, int line, Args &&...args) {
			LogImpl(level, file, line, FormatString(args...));
		}

		virtual void SwapBuffers() = 0; // Swap buffer
		virtual void ClearRecentEntries() = 0; // Clear the buffer
		virtual const std::deque<LogEntry>& GetEntries() const = 0;
		// Get Level
		virtual std::string GetLevelString(LogLevel level) const = 0;

	private:
		virtual void LogImpl(LogLevel level, const char* file, int line, const std::string& message) = 0;
	};

	LOG_API LogInterface* GetLogInstance();
}