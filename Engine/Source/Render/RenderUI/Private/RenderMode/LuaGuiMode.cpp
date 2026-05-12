#include <RenderMode/LuaGuiModeInternal.h>
#include <RenderMode/LuaGuiMode.h>

#include <ILog.h>
#include <Jobsystem/JobSystem.h>
#include <Theme.h>

//////////////////////////////
#include <AbsolutePath.h>
#include <Converter.h>
//////////////////////////////

#include <MagicUIContext.h>

namespace RenderUI {
    ILuaUIMember* GetLuaUIMember() {
        return &LuaUIMember::Get();
    }

    void LuaGuiMode::Draw()
    {
        if (AllowDraw.Allow() == false) { // 绘制
            Preprocessing();
            lua_getglobal(lua, "Draw");
            int ret = lua_pcall(lua, 0, 0, 0);
            if (ret != LUA_OK) {
                // 错误信息会在栈顶
                const char* err = lua_tostring(lua, -1);
                current_lua_error = err;
                LOG_ERROR("Lua Draw() Error:", current_lua_error);
                AllowDraw.isError = true;
                lua_pop(lua, 1);
            }
        }
        else {
            MteGUIContext& content = MteGUIContext::Get();
            //content.Label("Lua script error");
            content.BeginChild("LuaScriptError");
            content.DrawTextAligned(
                300, 200,
                current_lua_error,
                Theme::LOG_WARNING.x, Theme::LOG_WARNING.y, Theme::LOG_WARNING.z, Theme::LOG_WARNING.w,
                0.0f, 0.0f,
                0.0F, // fon size
                false
            );
            content.EndChild();
        }
    }

    void LuaGuiMode::LoadLua(std::string lua_type)
    {
        if (current_lua_type != lua_type) {
            std::wstring script_path = IO::AbsolutePath::Get().GetScriptPath();
            this->lua_path = IO::Converter::ToNarrowString(script_path) + "\\" + lua_type;
            current_lua_type = lua_type;

            sol::load_result result = lua.load_file(this->lua_path);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("Lua script compilation failed: ", err.what());
                AllowDraw.isCompileError = true;
                return;
            }
            else AllowDraw.isCompileError = false;
            lua_script = result;

            // 执行一次Lua
            lua_script();
            AllowDraw.isError = false;

            LOG_INFO("Lua script compilation completed: ", lua_type);
        }
    }

    void LuaGuiMode::UpdateLua()
    {
        if (!GetisLoading()) {
            AllowDraw.isLoading = true;
            Core::JobHandle handle = Core::JobSystem::Get().Schedule([this]() {
                LOG_INFO("Task Lua completed");
                {
                    sol::load_result result = lua.load_file(this->lua_path);
                    if (!result.valid()) {
                        sol::error err = result;
                        LOG_ERROR("Lua script compilation failed: ", err.what());
                        AllowDraw.isCompileError = true;
                        return;
                    }
                    else AllowDraw.isCompileError = false;
                    lua_script = result;lua_script();
                    AllowDraw.isError = false;
                    LOG_INFO("Lua script compilation completed");
                }
                AllowDraw.isLoading = false;
            });
        }
        else {
            LOG_WARNING("Please wait for the thread to finish loading Lua!");
        }
    }

    void LuaGuiMode::RuntimeError()
    {
        AllowDraw.isError = true;
        current_lua_error = "IMGUI internal error, please check the output log";
    }

    const bool LuaGuiMode::GetisLoading()
    {
        bool temp = AllowDraw.isLoading;
        return temp;
    }
    // --------------------------------------------- LuaUIMember ---------------------------------- //

    void LuaUIMember::Register(LuaMemberContext member)
    {
        LuaMember context(member.member, member.member_name);
        members.push_back(std::move(context));
    }

    const std::vector<LuaMember>& LuaUIMember::GetLuaMember()
    {
        return members;
    }
}