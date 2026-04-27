#pragma once 
#include <Common/EDITORWINDOWS_API.h>
#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>

namespace EditorWindows
{
	class EDITORWINDOWS_API Editor : public Core::Subsystem, public Singleton<Editor>
	{
	public:
		Editor();

		virtual bool Init();
		virtual void Uninstall();

		void CreateWindow();
	private:

	};

	struct AutoRegister;
	extern AutoRegister s_AutoRegister;
}