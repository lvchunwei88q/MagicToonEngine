#include <Core.h>
#include <iostream>

int main()
{
	Core core;
	std::cout << "Core version: " << Core::GetVersion() << std::endl;

	std::cin.get();
	return 0;
}