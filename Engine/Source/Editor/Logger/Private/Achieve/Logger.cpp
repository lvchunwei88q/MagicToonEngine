#include <Logger.h>
#include <FileManager.h>  
#include <AbsolutePath.h>
#include <Converter.h>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace LOG {
    AUTO_REGISTER(Logger)

    Logs* LOG::GetLogInstance()
    {
        return &Singleton<LOG::Logger>::Get();
    }

    bool Logger::Init()
    {
		std::wstring exepath = IO::AbsolutePath::Get().GetExecutableDirectory();
        logDir_ = exepath + L"\\" + CACHE + L"Logs";
        maxSizeMB_ = 100; // 100MB
        checkIntervalSec_ = 5; // 5s

        // 确保日志目录存在
        if (!IO::FileManager::Exists(logDir_)) {
            IO::FileManager::CreateDirectory(logDir_);
        }

        // 生成当前日志文件名（精确到秒）
        currentLogPath_ = logDir_ + L"/" + GenerateNewLogFileName();

        // 启动后台监控线程
        workerThread_ = std::make_unique<std::thread>(&Logger::BackgroundWorker, this);
        return true;
    }

    void Logger::Uninstall()
    {
        running_ = false;
        if (workerThread_ && workerThread_->joinable()) {
            workerThread_->join();
        }
        // 最后再刷新一次缓冲区
        FlushToFile();
    }

    void Logger::Log(LogLevel level, const char* file, int line, const std::string& message) {
        // 使用线程局部存储的 ostringstream，保证每次调用独立
        thread_local std::ostringstream oss;
        oss.str("");  // 清空内容
        oss.clear();

        // 格式：[时间] [级别] [文件:行号] 内容
        oss << "[" << GetCurrentTimestamp() << "] "
            << "[" << GetLevelString(level) << "] "
            << "[" << file << ":" << line   << "] "
            << message;
        std::string logLine = oss.str();

        WriteToDebugOutput(logLine);
        // 加入缓冲区（线程安全）
        {
            std::lock_guard<std::mutex> lock(fileMutex_);
            buffer_.push_back(logLine);
        }
    }

    std::string Logger::GetLevelString(LogLevel level) const {
        switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        default: return "UNKNOWN";
        }
    }

    std::string Logger::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm tm;
        localtime_s(&tm, &time_t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    std::wstring Logger::GenerateNewLogFileName() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
        localtime_s(&tm, &time_t);

        std::ostringstream oss;
        oss << "log_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".log";
        return IO::Converter::ToWideString(oss.str());
    }

    void Logger::WriteToFile(const std::string& text) {
        IO::FileManager::AppendText(currentLogPath_, text);
    }

    void Logger::BackgroundWorker() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(checkIntervalSec_));
            FlushToFile();
        }
    }

    void Logger::FlushToFile() {
        std::lock_guard<std::mutex> lock(fileMutex_);
        if (buffer_.empty()) return;

        // 将所有缓存行合并为一个字符串（每个行后加换行）
        std::string allLines;
        for (const auto& line : buffer_) {
            allLines += line + "\n";
        }

        // 写入文件（追加模式）
        IO::FileManager::AppendText(currentLogPath_, allLines);

        // 清空缓冲区
        buffer_.clear();
    }

    #include <Windows.h>       // OutputDebugString
    void Logger::WriteToDebugOutput(const std::string& text) {
        // 在 Windows 上输出到调试器
        OutputDebugStringA(text.c_str());
        OutputDebugStringA("\n");
    }

}