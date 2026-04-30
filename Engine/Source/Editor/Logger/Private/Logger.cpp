#include <Logger.h>
#include <FileManager.h>  
#include <AbsolutePath.h>
#include <Converter.h>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace LOG {
    AUTO_REGISTER(Logger)

    LogInterface* LOG::GetLogInstance()
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
            IO::FileManager::MakeDirectory(logDir_);
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

		cv_.notify_all(); // 通知线程退出
        if (workerThread_ && workerThread_->joinable()) {
            workerThread_->join();
        }
        // 最后再刷新一次缓冲区
        FlushToFile();
        ClearRecentEntries(); // 清空缓冲区
    }

    void Logger::Log(LogLevel level, const char* file, int line, const std::string& message) {
        // 使用线程局部存储的 ostringstream，保证每次调用独立
        thread_local std::ostringstream oss;
        oss.str("");  // 清空内容
        oss.clear();

        std::string time = GetCurrentTimestamp();

        // 格式：[时间] [级别] [文件:行号] 内容
        oss << "[" << time << "] "
            << "[" << GetLevelString(level) << "] "
            << "[" << file << ":" << line   << "] "
            << message;
        std::string logLine = oss.str();

        WriteToDebugOutput(logLine);
        {
            std::lock_guard<std::mutex> lock(Mutex_);
            // 加入缓冲区（线程安全）
            buffer_.push_back(logLine);
        
            // 加入环形缓冲区
            backBuffer_.push_back({ level, time, file, line, message });
        }
    }

    // 把新数据追加到 frontBuffer_
    void Logger::SwapBuffers() {
        std::lock_guard lock(Mutex_);

        // 把 backBuffer_ 的内容移动到 frontBuffer_ 末尾
        frontBuffer_.insert(
            frontBuffer_.end(),
            std::make_move_iterator(backBuffer_.begin()),
            std::make_move_iterator(backBuffer_.end())
        );
        backBuffer_.clear();

        // 超过上限就丢最旧的
        while (frontBuffer_.size() > MAX_RECENT_ENTRIES) {
            frontBuffer_.pop_front();
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
            std::unique_lock<std::mutex> lock(Mutex_);
            // 等待指定时间或被唤醒
            cv_.wait_for(lock, std::chrono::seconds(checkIntervalSec_),
                [this] { return !running_; });

            if (!running_) break;
            FlushToFile();
        }
    }

    void Logger::FlushToFile() {
        //std::lock_guard<std::mutex> lock(fileMutex_); 不持有锁
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

    void Logger::ClearRecentEntries() {
        std::lock_guard<std::mutex> lock(Mutex_);
        backBuffer_.clear();
        frontBuffer_.clear();
    }

}