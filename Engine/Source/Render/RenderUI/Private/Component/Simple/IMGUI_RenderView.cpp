
#include <Component/Simple/RenderView.h>

#include <IRenderRT.h>
#include <Component/EditorGeneralLayout.h>

#include <EditorUIWarehouse.h>

namespace RenderUI {
    RENDERUI_REGISTER(RenderView);

    void RenderView::Init()
    {
    }

    void RenderView::Uninstall()
    {
    }

    void RenderView::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.RenderViewWindow) {
            // Set style
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

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
                    IM_COL32(55, 55, 55, 255));  // 柔和色，完全不透明
            }

            ImGui::End();

            ImGui::PopStyleVar();
        }
    }

    void* RenderView::PublicData(uint8_t type)
    {
        return nullptr;
    }
}