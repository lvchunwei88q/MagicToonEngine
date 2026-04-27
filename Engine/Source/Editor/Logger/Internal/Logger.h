#pragma once
#include <Subsystem/SubsystemTemplate.h>
#include <Log.h>

#include <sstream>
#include <fstream>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>

namespace LOG {

    class Logger : public SubsystemTemplate<Logger, Core::SubsystemContext::Priority::High>, public LogInterface {
    public:
        //Logger() = default;

        virtual bool Init();
        virtual void Uninstall();

        // 获取流对象
        virtual void Log(LogLevel level, const char* file, int line, const std::string& message);

    private:

        void BackgroundWorker();        // 后台线程工作函数
        void FlushToFile();        // 将缓冲区内容写入文件并清空缓冲区
        void WriteToFile(const std::string& text);  // 写入文件
        void WriteToDebugOutput(const std::string& text); // 输出到调试器

        std::string GetLevelString(LogLevel level) const;
        std::string GetCurrentTimestamp() const;
        std::wstring GenerateNewLogFileName() const;  // 生成新的日志文件名（精确到秒）

        std::wstring logDir_;
        std::wstring currentLogPath_;    // 当前正在写入的日志文件路径
        size_t maxSizeMB_;
        unsigned int checkIntervalSec_;
        std::atomic<bool> running_{ true };
        std::unique_ptr<std::thread> workerThread_;
        std::mutex fileMutex_;          // 保护文件写入和滚动操作

        std::vector<std::string> buffer_;  // 缓存日志行
    };
};