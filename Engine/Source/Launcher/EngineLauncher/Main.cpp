#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <Editor.h>
#include <IProjectPath.h> // 检测当前是否为项目
#include <IProjectController.h>

#include <Window/WindowAppointment.hpp> // 通过约定确定Class
#include <iostream>

#ifdef _DEBUG
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        Core::SubsystemControl::Uninstall();
        return TRUE;  // 返回 TRUE 表示我们处理了这个事件
    default:
        return FALSE;
    }
}
#endif

//#include "test.h" // 测试用文件

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

int main(int argc, char* argv[])// exe -p path
{
#ifdef _DEBUG
    // console event
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#endif
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init ... " << std::endl;
    std::cout << argv[0] << argv[1] << argv[2] << argv[3] << std::endl;

    if (argc >= 3) {
        OperationType type = Operation(argv[1]); // exe_name , operation

        switch (type)
        {
        case OperationType::SpecifyProJect:
            {
                EngineProject::ProJectConfig config;
                config.path = argv[2];
                config.name = argv[3];
                EngineProject::GetProjectControllerInterface()->SpecifyProJect(config);
            }
            break;
        default:std::cout << "Operation Type Error!" << std::endl;
            break;
        }
    }

    if (!EngineProject::GetProjectPathInterface()->Detection()) { // 检测是否为项目
        // no!
        std::cout << "The engine has not yet opened a project!" << std::endl;
        std::cin.get();
        return 0;
    }

    (void)Editor::Editor::Get(); // 确保Editor初始化比Subsystem早

    // 注册初始化回调函数 发送给启动器(显示启动UI)
    Core::SubsystemControl::Register_Init_Callback([](const char* target, size_t len, size_t index) {
        float percent = (float)(index+1) / (float)Core::SubsystemControl::GetNum();
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

    Core::SubsystemContext::SubsystemError error = Core::SubsystemControl::Init();
    if (!error.error) {
        //每个进程有独立的虚拟地址空间，泄漏只影响自己的进程，不会污染其他程序或系统 所以不需要释放了
        std::cout << "There was a problem during engine initialization!" << std::endl;
        std::cout << "Problem Subsystem:" << error.target << std::endl;
        std::cin.get();
        return 0;
    }

    std::cout << "Engine initialized successfully. " << std::endl;

#ifdef _DEBUG
    // NOT
#else
    FreeConsole();
#endif
    Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Run);
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();
	return 0;
}