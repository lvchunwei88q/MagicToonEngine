#include <Component/DetailsWindow.h>
#include <MagicUIContext.h>
#include <ILog.h>

namespace RenderUI {
    AUTO_REGISTER(DetailsPanel);

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

	void DetailsWindow() // main Draw
	{
        if (Switch.DetailsWindow)
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin("Details Panel");
            {
                DetailsPanel::Get().Draw();
            }
            ImGui::End();
        }
	}
}