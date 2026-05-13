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
            auto result = lua.safe_script("Draw()",
                [](lua_State*, sol::protected_function_result pfr) {
                    // 出错时返回 pfr 本身，不抛异常
                    return pfr;
                }
            );
            if (!result.valid()) {
                sol::error err = result;
                current_lua_error = err.what();
                LOG_ERROR("Lua Draw() Error: ", current_lua_error);
                AllowDraw.isError = true;
            }
        }
        else {
            MteGUIContext& content = MteGUIContext::Get();
            //content.Label("Lua script error");
            content.BeginChild("LuaScriptError");
            content.DrawTextAligned(
                content.GetContentRegionAvailWidth(), content.GetContentRegionAvailHeight(),
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
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug); // base lib
        std::wstring ScriptPath = IO::AbsolutePath::Get().GetScriptPath();
        std::string narrowPath = IO::Converter::ToNarrowString(ScriptPath);
        {
            size_t pos = 0;
            while ((pos = narrowPath.find('\\', pos)) != std::string::npos) {
                narrowPath.replace(pos, 1, "\\\\");
                pos += 2;
            }
        }
        {
            std::string script = // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
                "package.path = package.path .. \";" +
                narrowPath +
                "/?.lua\"";
            auto result = lua.safe_script(script, sol::script_pass_on_error);
            if (!result.valid()) {
                sol::error err = result;
                LOG_ERROR("Lua Load Error: ", err.what());
            }
        }

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
            auto exec_result = lua_script();
            if (!exec_result.valid()) {
                sol::error err = exec_result;
                LOG_ERROR("Lua script execution failed: ", err.what());
                AllowDraw.isError = true;
            }
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