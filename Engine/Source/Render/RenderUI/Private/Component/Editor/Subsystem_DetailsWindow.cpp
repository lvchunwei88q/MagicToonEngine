#include <Component/Editor/DetailsWindow.h>
#include <MagicUIContext.h>
#include <ILog.h>

#include <Component/Editor/EditorGeneralLayout.h>


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