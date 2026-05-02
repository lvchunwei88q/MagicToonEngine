#include <GuiInterface.h>
#include <RenderRTI.h>

namespace RenderUI {
	void RenderViewWindow() {
		if (Switch.RenderViewWindow) {

            // Save Current style
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec2 originalWindowPadding = style.WindowPadding;

            // set this windows padding
            style.WindowPadding = ImVec2(0.0f, 0.0f);

            // set windows flags
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse;

            ImGui::Begin("Render View", nullptr, windowFlags);
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            RenderRT::GetRenderRTInterface()->UpdateBufferManagerViewSize((int)windowSize.x, (int)windowSize.y);

            // Example
            {
                ImVec2 renderPos = ImGui::GetCursorScreenPos();
                ImDrawList* drawList = ImGui::GetWindowDrawList();

                // 绘制红色背景填充
                drawList->AddRectFilled(renderPos,
                    ImVec2(renderPos.x + windowSize.x, renderPos.y + windowSize.y),
                    IM_COL32(255, 0, 0, 255));  // 纯红色，完全不透明
            }

            ImGui::End();

            style.WindowPadding = originalWindowPadding;
		}
	}
}