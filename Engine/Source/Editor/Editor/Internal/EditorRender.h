#pragma once
#include <EditorUI.h>
#include <Tools/Singleton.h>
#include <IRender.h> // 呈现

using namespace RenderEditor;

namespace Editor {
	class EditorRender : public Singleton<EditorRender>
	{
	public:
		void Tick();
		void Init();
		void End();

	private:
		EditorUI editorUI;
	};
}