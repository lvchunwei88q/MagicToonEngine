#include <windows.h>
#include "Main.h"
#include <Editor.h>

#include "../Test/test.h" // 测试用文件

int main(int argc, char* argv[])// exe -p path
{

    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init ... " << std::endl;

    if (!EnginePreInit(argc, argv)) { return 0; }

    Core::SubsystemError error = Core::SubsystemControl::Init();
    if (!error.error) {
        //每个进程有独立的虚拟地址空间，泄漏只影响自己的进程，不会污染其他程序或系统 所以不需要释放了
        std::cout << "There was a problem during engine initialization!" << std::endl;
        std::cout << "Problem Subsystem:" << error.target << std::endl;
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        std::cin.get();
        return 0;
    }

    std::cout << "Engine initialized successfully. " << std::endl;

    FreeConsole(); // Free Console
    Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Run);
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();
	return 0;
}