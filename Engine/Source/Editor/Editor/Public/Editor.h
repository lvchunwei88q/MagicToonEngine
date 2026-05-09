#pragma once

#include <Common/EDITOR_API.h>
#include <Tools/Singleton.h>

#include <Windows/Windows.h>

namespace Editor
{
	class EDITOR_API Editor : public Singleton<Editor>
	{
	public:
		Editor();

		enum class EngineState {
			Run = 0,
			Stop = 1,
		};

		void InitCoreLogCapture();
		void SetEngineState(EngineState State);

		void Run();

	private:
		//EditorWindows::EditorWindows* editorWindows_ = &EditorWindows::EditorWindows::Get();
		EngineState state;
		void Tick();
	};
}