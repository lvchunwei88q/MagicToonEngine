#include <Core.h>
#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <iostream>

class MyClass : public SubsystemTemplate<MyClass>
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
AUTO_REGISTER(MyClass);

int main()
{
	Core::Core core;
	std::cout << "Core version: " << Core::Core::GetVersion() << std::endl;

	Core::SubsystemControl::Init();
	Core::SubsystemControl::Uninstall();

	std::cin.get();
	return 0;
}