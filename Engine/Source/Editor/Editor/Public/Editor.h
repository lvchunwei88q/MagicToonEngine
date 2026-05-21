#pragma once

#include "Common/EDITOR_API.h"
#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>

namespace Editor
{
	class EDITOR_API Editor : public Singleton<Editor> , public Core::Subsystem
	{
	public:
		Editor() = default;

		virtual bool Init();
		virtual void Uninstall();

		enum class EngineState {
			Run = 0,
			Stop = 1,
		};

		void SetEngineState(EngineState State);

		void Run();

	private:
		//EditorWindows::EditorWindows* editorWindows_ = &EditorWindows::EditorWindows::Get();
		EngineState state;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(Editor);
}