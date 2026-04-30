#pragma once

#include <Common/LOG_API.h>

#include <string>

#include <deque>  // 环形缓冲区

#define IN_SPECIAL_NAMESPACE

#ifdef IN_SPECIAL_NAMESPACE
#define LOGSPACE LOG
#else
#define LOGSPACE
#endif

// 宏定义（方便使用）
#define LOG_DEBUG(msg)   LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Debug,__FILE__,__LINE__,msg) 
#define LOG_INFO(msg)    LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Info,__FILE__,__LINE__,msg) 
#define LOG_WARNING(msg) LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Warning,__FILE__,__LINE__,msg) 
#define LOG_ERROR(msg)   LOGSPACE::GetLogInstance()->Log(LOGSPACE::LogLevel::Error,__FILE__,__LINE__,msg) 

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
	public:
		LogInterface() = default;
		// 获取流对象
		virtual void Log(LogLevel level, const char* file, int line, const std::string& message) = 0;
		virtual const std::deque<LogEntry>& GetEntries() const = 0;
	};

	LOG_API LogInterface* GetLogInstance();
}

#undef IN_SPECIAL_NAMESPACE