#pragma once
#include <Object.h>
#include <Windows.h>
#include <Tools/Singleton.h>

#include <WindowsConfig.object.generate.h>

namespace Editor {

	MCLASS(MSERIALIZATION);
	class WindowsConfig : public MObject
	{
		GENERATE_BODY(WindowsConfig, ENGINE, "Windows");
	public:
		// This Windows Size
		MMEMBER();
		int2 WindowsSize = int2(1280,800);

		// This Windows Postion
		MMEMBER();
		int2 WindowsPostion = int2(100, 100);
	};
}