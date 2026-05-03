#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <Editor.h>
#include <iostream>

int main()
{
    std::cout << "Engine Version: " << Core::Core::GetVersion() << std::endl;
    std::cout << "Engine Init ... " << std::endl;

    Core::SubsystemContext::SubsystemError error = Core::SubsystemControl::Init();
    if (!error.error) {
        //每个进程有独立的虚拟地址空间，泄漏只影响自己的进程，不会污染其他程序或系统 所以不需要释放了
        std::cout << "There was a problem during engine initialization!" << std::endl;
        std::cout << "Problem Subsystem:" << error.target << std::endl;
        std::cin.get();
        return 0;
    }

    std::cout << "Engine initialized successfully with no errors. " << std::endl;
    FreeConsole();

    Editor::Editor::Get().SetEngineState(Editor::Editor::EngineState::Run);
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();
	return 0;
}