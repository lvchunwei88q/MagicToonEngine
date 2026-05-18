#include "Component/Editor/DetailsWindow.h"
#include "Component/Editor/EditorGeneralLayout.h"
#include "EditorUIWarehouse.h"

namespace RenderUI {
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