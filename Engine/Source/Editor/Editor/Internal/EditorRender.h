#pragma once
#include <RenderUI.h>
#include <Common/Singleton.h>
#include <RenderInterface.h> // 呈现

namespace Editor {
	class EditorRender : public Singleton<EditorRender>
	{
	public:
		void Tick();
		void Init();
		void End();

	private:
		RenderUI::RenderUI renderui;
	};
}