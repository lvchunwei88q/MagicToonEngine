#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Editor.h>
#include <iostream>

int main()
{
	std::cout << "Version: " << Core::Core::GetVersion() << std::endl;

	Core::SubsystemControl::Init();
	Editor::Editor::Get().Run(); // Run Engine Editor

	Core::SubsystemControl::Uninstall();
	return 0;
}