#include "Editor/DetailsWindow.h"
#include "Editor/EditorGeneralLayout.h"
#include <RenderUIWarehouse.h>

namespace RenderEditor {
    void DetailsPanel::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.DetailsWindow)
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin(WINDOWS_NAME);
            {
                DetailsPanel::Get().Draw();
            }
            ImGui::End();
        }
    }
}