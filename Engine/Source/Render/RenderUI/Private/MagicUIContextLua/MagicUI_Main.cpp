#include "MagicUIContext.h"

namespace RenderUI {
    void MteGUIContext::BindLuaFunction(sol::state& lua) {
        MteGUIContext& content = MteGUIContext::Get();
        lua.new_usertype<MteGUIContext>("GUI", sol::no_constructor);

        BindCore(lua, "GUI");
        BindLayout(lua, "GUI");
        BindWindowPopup(lua, "GUI");
        BindTableDraw(lua, "GUI");
        BindUtils(lua, "GUI");

        lua["GUI"] = &content;
    }
}