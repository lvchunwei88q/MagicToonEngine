
#include <RenderUIWarehouse.h>
#include "EditorUI.h"
#include <RenderUI.h>

#include <ILog.h>

namespace RenderEditor {

	LRESULT EditorUI_IMGUI_Event(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return RenderUI::RenderUI_IMGUI_Windows_Event(hWnd, msg, wParam, lParam);
	}

	void EditorUI::Init()
	{
		ImGuiContext* ctx = RenderUI::RenderUIManager::Get().GetImGuiContext();
		if (ctx) {
			ImGui::SetCurrentContext(ctx);  // 设置到当前模块
		}else
		{
			LOG_ERROR("Failed to obtain the current IMGUI context, UI cannot be rendered");
		}
	}

	void EditorUI::Shutdown()
	{

	}

	void EditorUI::Tick()
	{
		RenderUI::RenderUIManager::Get().Tick();
	}
}