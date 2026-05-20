#pragma once
#include <windows.h>
#include <EditorUI.h>
#include <Tools/Singleton.h>
#include <IRender.h> // 呈现

namespace EngineLauncher {
	class LauncherRender : public Singleton<LauncherRender>
	{
	public:
		void Tick();
		void Init();
		void End();

	private:
		RenderUI::EditorUI editorUI;
	};
}