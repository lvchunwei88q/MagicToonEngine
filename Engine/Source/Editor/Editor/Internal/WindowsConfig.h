#pragma once
#include <Object.h>
#include <IO.h>
#include <Windows.h>
#include <Tools/Singleton.h>

#include <WindowsConfig.object.generate.h>

namespace Editor {

	MCLASS(MSERIALIZATION);
	class WindowsConfig : public MObject
	{
		GENERATE_BODY(WindowsConfig, ENGINE, "Windows");
	protected:
		std::wstring GetObjectCustomSerializedPath() override {
			return IO::AbsolutePath::Get().GetExecutableDirectory() + L"\\Config\\Editor\\WindowsConfig.mtdata";
		}
	public:
		// This Windows Size
		MMEMBER();
		int2 WindowsSize = int2(1280,800);

		// This Windows Postion
		MMEMBER();
		int2 WindowsPostion = int2(100, 100);
	};
}