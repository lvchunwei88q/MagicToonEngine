#include "Editor/LuaUIControl.h"
#include <MagicUIContext.h>
#include "Editor/EditorGeneralLayout.h"

#include <ILog.h>
#include <Jobsystem/JobSystem.h>
#include <Theme.h>
//////////////////////////////
#include <IO.h> 
//////////////////////////////

namespace RenderEditor {
    RENDERUI_REGISTER(LuaUIControlWindows);

    void LuaUIControlWindows::Init()
    {
        REGISTER_LUA_MEMBER(&LuaUIControlWindows::Get(), "LuaUI Control");
        RegisterLua("EditorUI\\LuaControlPanel.lua");
        MteGUIContext::Get().BindLuaFunction(lua);

        sol::table persistent = lua["PersistentData"].get_or_create<sol::table>();
        lua.set_function("Update", [](std::string lua_type, std::string target) {
            const std::vector<LuaMember>& LuaMembers = GetLuaUIMember()->GetLuaMember();
            for (auto& LuaMember : LuaMembers)
            {
                if (LuaMember.member_name == target || LuaMember.member->GetLuaType() == lua_type) {
                    if (!LuaMember.member->GetisLoading())
                        LuaMember.member->UpdateLua();
                    else {
                        LOG_WARNING("Please wait for the thread to finish loading Lua!");
                    }
                }
            }
        });
    }
    void LuaUIControlWindows::Uninstall()
    {
    }

    void* LuaUIControlWindows::PublicData(uint8_t type)
    {
        return nullptr;
    }

    void LuaUIControlWindows::Notification(const char* msg)
    {
        if (strstr(msg, WINDOWS_NAME) != nullptr) {
            RuntimeError();
        }
    }

    void LuaUIControlWindows::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.LuaControl)
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin(WINDOWS_NAME);
            {
                Draw();
            }
            ImGui::End();
        }
    }

    void LuaUIControlWindows::Preprocessing()
    {
        const std::vector<LuaMember>& LuaMembers = GetLuaUIMember()->GetLuaMember();

        sol::table tbl = lua.create_table();
        for (size_t i = 0; i < LuaMembers.size(); ++i)
        {
            std::string name = LuaMembers[i].member_name;
            std::string ltype = LuaMembers[i].member->GetLuaType();

            sol::table m = lua.create_table();
            m["member_name"] = name;
            m["lua_type"] = ltype;
            tbl[i] = m;  // 1-based
        }

        lua["LuaMembers"] = tbl;
        lua["LuaMembersCount"] = (int)LuaMembers.size();
    }
}