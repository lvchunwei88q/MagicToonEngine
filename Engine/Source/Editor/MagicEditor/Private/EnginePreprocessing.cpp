#include "Main.h"
#include <cmdline/cmdline.h>
#include <Windows.h>

#include <IProjectPath.h> // 检测当前是否为项目
#include <IProjectController.h>
#include <IO.h> 

#include <Window/WindowAppointment.hpp> // 通过约定确定Class

bool CommandCheck(const std::vector<std::wstring>& argsW) {
    cmdline::parser parser;
    std::vector<std::string> args;

    parser.add<std::string>("project_path", 'p', "Project path", true, "");
    parser.add<std::string>("name", 'n', "Project name", true, "");
    parser.add("help", '?', "Print help");

    for (auto& argw : argsW) {
        args.push_back(IO::ToNarrowString(argw));
    }

    if (parser.parse_check(args)) {
        std::string ProjectDir = parser.get<std::string>("project_path");
        std::string ProjectName = parser.get<std::string>("name");

        EngineProject::ProJectConfig config;
        config.path = ProjectDir;
        config.name = ProjectName;
        EngineProject::GetProjectControllerInterface()->SpecifyProJect(config);
    }
    else {
        // 是否存在历史项目
        EngineProject::GetProjectControllerInterface()->LoadHistoryProjects();
    }

    if (!EngineProject::GetProjectPathInterface()->Detection()) { // 检测是否为项目
        return false;
    }

    return true;
}

void RunMagicLauncher() {
    // 构建 ENGINE_LAUNCHER_NAME_W 路径
    std::wstring exeDir = IO::AbsolutePath::Get().GetExecutableDirectory();
    std::wstring exePath = exeDir + L"\\" ENGINE_LAUNCHER_NAME_W L".exe";

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
}

void RegisterSubsystemCallback()
{
    // 注册初始化回调函数 发送给启动器
    Core::SubsystemControl::Register_Init_Callback([](const char* target, size_t len, size_t index) {
        float percent = (float)(index + 1) / (float)Core::SubsystemControl::GetNum();
        int char_len = (int)len;// const char*

        std::vector<uint8_t> data;
        data.insert(data.end(), (uint8_t*)&char_len, (uint8_t*)&char_len + sizeof(int));
        data.insert(data.end(), (uint8_t*)&percent, (uint8_t*)&percent + sizeof(float));
        data.insert(data.end(), target, target + len);

        static HWND hTarget = FindWindowW(ENGINE_LAUNCHER_CLASS, nullptr);
        if (!hTarget) return;

        COPYDATASTRUCT cds;
        cds.dwData = MessageTypeNumber_0;
        cds.cbData = sizeof(int) + sizeof(float) + char_len;
        cds.lpData = data.data();
        SendMessageW(hTarget, WM_COPYDATA, 0, (LPARAM)&cds);
    });
}