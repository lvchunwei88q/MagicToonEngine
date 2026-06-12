#include "Main.h"

#include <Windows.h>

#include <IProjectPath.h> // 检测当前是否为项目
#include <IProjectController.h>

#include <IO.h> 
#include <vector>

#include <Window/WindowAppointment.hpp> // 通过约定确定Class


enum class OperationType {
    NOT,
    SpecifyProJect
};

OperationType Operation(const std::string& o) {
    // -p | -project

    if (o == "-p" || o == "-project") {
        return OperationType::SpecifyProJect;
    }
    else {
        return OperationType::NOT;
    }
}

bool EngineCheck(int argc, wchar_t* argv[]) {
    std::wstring wArgv = argv[1] ? argv[1] : L"NOT";
    std::string Argv = IO::ToNarrowString(wArgv);
    OperationType type = Operation(Argv); // operation
    switch (type)
    {
    case OperationType::SpecifyProJect:
    {
        EngineProject::ProJectConfig config;
        config.path = IO::ToNarrowString(argv[2]);
        config.name = IO::ToNarrowString(argv[3]);
        EngineProject::GetProjectControllerInterface()->SpecifyProJect(config);
    }
    break;
    case OperationType::NOT:
    {
        // 是否存在历史项目
        EngineProject::GetProjectControllerInterface()->LoadHistoryProjects();
    }
    break;
    default:std::cout << "Operation Type Error!" << std::endl;
        break;
    }

    if (!EngineProject::GetProjectPathInterface()->Detection()) { // 检测是否为项目
        // no!
        std::cout << "The engine has not yet opened a project!" << std::endl;
        // 构建 MagicLauncher.exe 路径
        std::wstring exeDir = IO::AbsolutePath::Get().GetExecutableDirectory();
        std::wstring exePath = exeDir + L"\\MagicLauncher.exe";

        std::wstring cmdLine = L"\"" + exePath + L"\"";

        // 启动进程
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};
        BOOL success = CreateProcessW(
            nullptr,
            &cmdLine[0],
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        );
        return false;
    }

    // 注册初始化回调函数 发送给启动器(显示启动UI)
    Core::SubsystemControl::Register_Init_Callback([](const char* target, size_t len, size_t index) {
        float percent = (float)(index + 1) / (float)Core::SubsystemControl::GetNum();
        int char_len = (int)len;// const char*

        std::vector<uint8_t> data;
        data.insert(data.end(), (uint8_t*)&char_len, (uint8_t*)&char_len + sizeof(int));
        data.insert(data.end(), (uint8_t*)&percent, (uint8_t*)&percent + sizeof(float));
        data.insert(data.end(), target, target + len);

        static HWND hTarget = FindWindowW(PROJECT_CLASS, nullptr);
        if (!hTarget) return;

        COPYDATASTRUCT cds;
        cds.dwData = MessageTypeNumber_0;
        cds.cbData = sizeof(int) + sizeof(float) + char_len;
        cds.lpData = data.data();
        SendMessageW(hTarget, WM_COPYDATA, 0, (LPARAM)&cds);
    });

    return true;
}