#pragma once
#include <Object.h>
#include <Windows.h>
#include <Tools/Singleton.h>

#include <WindowsConfig.object.generate.h>

namespace Editor {

	MCLASS(MSERIALIZATION);
	class WindowsConfig : public Core::Object
	{
		GENERATE_BODY(WindowsConfig, ENGINE, "Windows");
	public:
		// This Windows Size
		MMEMBER();
		int width = 1280;
		MMEMBER();
		int height = 800;

		// This Windows Postion
		MMEMBER();
		int windowsX = 100;
		MMEMBER();
		int windowsY = 100;
	};
}