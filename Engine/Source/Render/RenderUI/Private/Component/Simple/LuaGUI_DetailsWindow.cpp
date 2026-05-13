#include <Component/Simple/DetailsWindow.h>
#include <MagicUIContext.h>
#include <ILog.h>

#include <Component/EditorGeneralLayout.h>

namespace RenderUI {
    RENDERUI_REGISTER(DetailsPanel);

    void DetailsPanel::Init()
    {
        RegisterLua("EditorUI\\JsonPanel.lua");
        MteGUIContext::Get().BindLuaFunction(lua);

        REGISTER_LUA_MEMBER(&DetailsPanel::Get(), "Json Panel");
    }

    void DetailsPanel::Uninstall()
    {
    }

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

    void* DetailsPanel::PublicData(uint8_t type)
    {
        return nullptr;
    }

    void DetailsPanel::Notification(const char* msg)
    {
        if (strstr(msg, WINDOWS_NAME) != nullptr) {
            RuntimeError();
        }
    }
}