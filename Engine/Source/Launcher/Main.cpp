#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Editor.h>
#include <iostream>

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_CLOSE_EVENT ||
        signal == CTRL_C_EVENT ||
        signal == CTRL_BREAK_EVENT ||
        signal == CTRL_LOGOFF_EVENT ||
        signal == CTRL_SHUTDOWN_EVENT) {
        Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Stop);
        Core::SubsystemControl::Uninstall();  // 主动清理
        return TRUE;  // 表示已处理
    }
    return FALSE;
}

int main()
{
    // 注册控制台事件处理
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	std::cout << "Version: " << Core::Core::GetVersion() << std::endl;

	Core::SubsystemControl::Init();
    Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Run);
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();
	return 0;
}