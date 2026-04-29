#pragma once

#include <Common/Core_API.h>
#include <string>

namespace Core
{
    /*
	* 错误捕获是允许不依赖或者在系统日志直接定义的模块使用，它最终会使用日志系统来记录错误信息。
	* 所以它的面向对象就是Core模块，因为Core模块无法引用日志模块。
    */
    class CORE_API ErrorCapture {
    public:
        using FuncType = void(*)(const std::string& errorMessage);
    private:
        static FuncType GetCaptureFunction;
    public:
        static void RegisterCaptureFunction(FuncType func);

        static void Capture(const std::string& errorMessage);
    };
}