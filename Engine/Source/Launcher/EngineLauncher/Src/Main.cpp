#include "Main.h"
#include <Editor.h>

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

int main(int argc, char* argv[])// exe -p path
{
#ifdef _DEBUG
    // console event
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#endif

    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init ... " << std::endl;

    if (!EnginePreInit(argc, argv)) { return 0; }
    (void)Editor::Editor::Get(); // 确保Editor初始化比Subsystem早

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