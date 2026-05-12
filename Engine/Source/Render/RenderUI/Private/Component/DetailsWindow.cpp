#include <Component/DetailsWindow.h>
#include <MagicUIContext.h>
#include <ILog.h>

#define WINDOWS_NAME "Details Panel"

namespace RenderUI {
    AUTO_REGISTER_NOTIFICATION(DetailsPanel,"IMGUI");

    bool DetailsPanel::Init()
    {
        MteGUIContext::Get().BindLuaFunction(lua);
        LoadLua("EditorUI\\JsonPanel.lua");
        REGISTER_LUA_MEMBER(&DetailsPanel::Get(), "Details Panel");
        return true;
    }

    void DetailsPanel::Uninstall()
    {
    }

    void DetailsPanel::Notification(const char* msg)
    {
        if (strstr(msg, WINDOWS_NAME) != nullptr) {
            RuntimeError();
        }
    }

	void DetailsWindow() // main Draw
	{
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