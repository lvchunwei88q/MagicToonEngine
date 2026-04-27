#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>

#include <iostream>
#include <Editor.h>

class MyClass : public SubsystemTemplate<MyClass, Core::SubsystemContext::Priority::Normal>
{
public:
	virtual bool Init() {
		std::cout << "MyClass Init" << std::endl;
		return true;
	}
	virtual void Uninstall() {
		std::cout << "MyClass Uninstall" << std::endl;
	}
private:

};
AUTO_REGISTER(MyClass)

int main()
{
	Core::Core core;
	std::cout << "version: " << Core::Core::GetVersion() << std::endl;

	Core::SubsystemControl::Init();

	EditorWindows::Editor::Get().CreateWindow(); // Example usage of the Editor subsystem

	Core::SubsystemControl::Uninstall();

	std::cin.get();
	return 0;
}