#include "Editor/DetailsWindow.h"
#include <MagicUIContext.h>
#include <ILog.h>

#include "Editor/EditorGeneralLayout.h"

namespace RenderEditor {
    RENDERUI_REGISTER(DetailsPanel);

    namespace {
        std::shared_ptr<LuaMemberClass> CreateNewMember(const std::string& name, const std::string& type) {
            auto member = std::make_shared<LuaMemberClass>();
            member->member_name = name;
            member->lua_type = type;
            return member;
        }
    }

    void DetailsPanel::Init()
    {
        REGISTER_LUA_MEMBER(&DetailsPanel::Get(), "Json Panel");

        RegisterLua("EditorUI\\JsonPanel.lua");
        MteGUIContext::Get().BindLuaFunction(lua);

        lua.new_usertype<LuaMemberClass>("LuaMemberClass",
            // 构造函数
            sol::constructors<LuaMemberClass()>(),

            "member_name", & LuaMemberClass::member_name,
            "lua_type", & LuaMemberClass::lua_type,

            "Print", & LuaMemberClass::Print
        );

        lua.set_function("CreateNewMember", CreateNewMember);
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