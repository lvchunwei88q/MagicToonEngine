#include <Component/LuaUIControl.h>
#include <MagicUIContext.h>

#include <ILog.h>
#include <Jobsystem/JobSystem.h>
#include <Theme.h>
//////////////////////////////
#include <AbsolutePath.h>
#include <Converter.h>
//////////////////////////////

namespace RenderUI {
    AUTO_REGISTER(LuaUIControlWindows);

    LuaMember::LuaMember(LuaUIControl* m, std::string n) : member(m), member_name(n)
    {
        lua_type = m->GetLuaType();
    }

	void LuaUIControl::Draw()
	{
		if (isDraw.is() == false) { // 绘制
            Preprocessing();
            lua_getglobal(lua, "Draw");
            int ret = lua_pcall(lua, 0, 0, 0);
            if (ret != LUA_OK) {
                // 错误信息会在栈顶
                const char* err = lua_tostring(lua, -1);
                current_lua_error = err;
                LOG_ERROR("Lua Draw() Error:", current_lua_error);
                isDraw.isError = true;
                lua_pop(lua, 1);
            }
        }
        else {
            MteGUIContext& content = MteGUIContext::Get();
            //content.Label("Lua script error");
            content.BeginChild("LuaScriptError");
            content.DrawTextAligned(
                300,200,
                current_lua_error,
                Theme::LOG_WARNING.x, Theme::LOG_WARNING.y, Theme::LOG_WARNING.z, Theme::LOG_WARNING.w,
                0.0f, 0.0f,
                0.0F, // fon size
                false
            );
            content.EndChild();
        }
	}

    void LuaUIControl::LoadLua(std::string lua_type)
    {
        if (current_lua_type != lua_type) {
            std::wstring script_path = IO::AbsolutePath::Get().GetScriptPath();
            this->lua_path = IO::Converter::ToNarrowString(script_path) + "\\" + lua_type;
            current_lua_type = lua_type;

            sol::load_result result = lua.load_file(this->lua_path);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("Lua script compilation failed: ", err.what());
                isDraw.isCompileError = true;
                return;
            }
            else isDraw.isCompileError = false;
            lua_script = result;

            // 执行一次Lua
            lua_script();
            isDraw.isError = false;

            LOG_INFO("Lua script compilation completed: ", lua_type);
        }
    }

    void LuaUIControl::UpdateLua()
    {
        isDraw.isLoading = true;
        Core::JobHandle handle = Core::JobSystem::Get().Schedule([this]() {
            LOG_INFO("Task Lua completed");
            {
                sol::load_result result = lua.load_file(this->lua_path);
                if (!result.valid()) {
                    sol::error err = result;
                    LOG_ERROR("Lua script compilation failed: ", err.what());
                    isDraw.isCompileError = true;
                    return;
                }
                else isDraw.isCompileError = false;
                lua_script = result;lua_script();
                isDraw.isError = false;
                LOG_INFO("Lua script compilation completed");
            }
            isDraw.isLoading = false;
        });
    }

    const bool LuaUIControl::GetisLoading()
    {
        bool temp = isDraw.isLoading;
        return temp;
    }

    void LuaUIMember::Register(LuaMember member)
    {
        members.push_back(std::move(member));
    }

    const std::vector<LuaMember>& LuaUIMember::GetLuaMember()
    {
        return members;
    }

    // ----------------------------------------------------- UI ---------------------------------------------------------//
    bool LuaUIControlWindows::Init()
    {
        MteGUIContext::Get().BindLuaFunction(lua);
        LoadLua("EditorUI\\LuaControlPanel.lua");
        REGISTER_LUA_MEMBER(&LuaUIControlWindows::Get(), "LuaUI Control");

        lua.set_function("Update", [](std::string lua_type, std::string target) {
            const std::vector<LuaMember>& LuaMembers = LuaUIMember::Get().GetLuaMember();
            for (auto& LuaMember: LuaMembers)
            {
                if (LuaMember.member_name == target || LuaMember.lua_type == lua_type) {
                    if(!LuaMember.member->GetisLoading())
                        LuaMember.member->UpdateLua();
                    else {
                        LOG_WARNING("Please wait for the thread to finish loading Lua!");
                    }
                }
            }
        });
        return true;
    }
    void LuaUIControlWindows::Uninstall()
    {
    }

    void LuaUIControlWindows::Preprocessing()
    {
        const std::vector<LuaMember>& LuaMembers = LuaUIMember::Get().GetLuaMember();

        sol::table tbl = lua.create_table();
        for (size_t i = 0; i < LuaMembers.size(); ++i)
        {
            std::string name = LuaMembers[i].member_name;
            std::string ltype = LuaMembers[i].lua_type;

            sol::table m = lua.create_table();
            m["member_name"] = name;
            m["lua_type"] = ltype;
            tbl[i] = m;  // 1-based
        }

        lua["LuaMembers"] = tbl;
        lua["LuaMembersCount"] = (int)LuaMembers.size();
    }

    void LuaUIControlWindow() // main Draw
    {
        if (Switch.LuaControl)
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin("LuaControl Panel");
            {
                LuaUIControlWindows::Get().Draw();
            }
            ImGui::End();
        }
    }
}