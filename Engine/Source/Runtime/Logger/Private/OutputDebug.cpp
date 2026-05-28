#include <Windows.h>

#include "LoggerImpl.h"

namespace LOG {
    void LoggerImpl::WriteToDebugOutput(const std::string& text) {
        // 在 Windows 上输出到调试器
        OutputDebugStringA(text.c_str());
        OutputDebugStringA("\n");
    }
}