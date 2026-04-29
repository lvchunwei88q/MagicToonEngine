#pragma once

#include <Common/EDITOR_API.h>
#include <Common/Singleton.h>

#include <EditorWindows.h>

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

		void InitCoreErrorCapture();
		void SetEngineState(EngineState State);

		void Run();

	private:
		//EditorWindows::EditorWindows* editorWindows_ = &EditorWindows::EditorWindows::Get();
		EngineState state;
		void Tick();
	};
}