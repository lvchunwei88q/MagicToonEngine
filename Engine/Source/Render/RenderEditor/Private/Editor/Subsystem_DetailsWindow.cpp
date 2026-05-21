#include "Editor/DetailsWindow.h"
#include <MagicUIContext.h>
#include <ILog.h>

#include "Editor/EditorGeneralLayout.h"

namespace RenderEditor {
    RENDERUI_REGISTER(DetailsPanel);

    void DetailsPanel::Init()
    {
        REGISTER_LUA_MEMBER(&DetailsPanel::Get(), "Json Panel");

        RegisterLua("EditorUI\\JsonPanel.lua");
        MteGUIContext::Get().BindLuaFunction(lua);
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