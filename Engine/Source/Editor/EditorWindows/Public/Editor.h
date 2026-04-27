#pragma once 
#include <Common/EDITORWINDOWS_API.h>
#include <Subsystem/Subsystem.h>
#include <Common/Singleton.h>

#include <Subsystem/SubsystemTemplate.h>

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

	AUTO_REGISTER_SINGLETON_INCLUDE(Editor)
}