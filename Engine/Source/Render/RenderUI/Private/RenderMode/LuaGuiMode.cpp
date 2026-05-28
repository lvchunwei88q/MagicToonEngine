#include "RenderMode/LuaGuiModeInternal.h"
#include "RenderMode/LuaGuiMode.h"

#include <ILog.h>
#include <Jobsystem/JobSystem.h>
#include <Theme.h>

//////////////////////////////
#include <IO.h> 
//////////////////////////////

#include "MagicUIContext.h"

namespace RenderUI {
    namespace {
        class RAIIAtomicBool {
        public:
            std::atomic<bool>& src;
            bool target;
            RAIIAtomicBool(std::atomic<bool>& src,bool target) : src(src) {
                this->target = target;
            }
            ~RAIIAtomicBool() {
				src = target;
            }
        };
    }

    ILuaUIMember* GetLuaUIMember() {
        return &LuaUIMember::Get();
    }

    void LuaGuiMode::Draw()
    {
        if (AllowDraw.Allow() == false) { // 绘制
            Preprocessing();
            auto result = lua.safe_script("Draw()", sol::script_pass_on_error);
            if (!result.valid()) {
                RuntimeError(result);
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

    void LuaGuiMode::RegisterLua(std::string lua_type)
    {
        lua = sol::state();
        lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::debug); // base lib

        std::wstring ScriptPath = IO::AbsolutePath::Get().GetScriptPath();
        std::string narrowPath = IO::ToNarrowString(ScriptPath);
        std::replace(narrowPath.begin(), narrowPath.end(), '\\', '/');
        {
            std::string script =
                "package.path = package.path .. \";" +
                narrowPath +
                "/?.lua\"";
            auto result = lua.safe_script(script, sol::script_pass_on_error);
            if (!result.valid()) {
                RuntimeError(result);
                return;
            }
        }

        this->lua_path = IO::ToNarrowString(ScriptPath) + "\\" + lua_type;
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
            RuntimeError(exec_result);
            return;
        }
        AllowDraw.isError = false;

        LOG_INFO("Lua script compilation completed: ", lua_type);
    }

    void LuaGuiMode::UpdateLua()
    {
        if (!GetisLoading()) {
            AllowDraw.isLoading = true;
            Core::JobHandle handle = Core::JobSystem::Get().Schedule([this]() {
                LOG_INFO("Task Lua completed");
                {
                    RAIIAtomicBool _isLoading(AllowDraw.isLoading, false);
                    // lua_load
                    sol::load_result result = lua.load_file(this->lua_path);
                    if (!result.valid()) {
                        sol::error err = result;
                        LOG_ERROR("Lua script compilation failed: ", err.what());
                        AllowDraw.isCompileError = true;
                        return;
                    }
                    else AllowDraw.isCompileError = false;
                    lua_script = result;
                    // lua_script
                    auto exec_result = lua_script();
                    if (!exec_result.valid()) {
                        RuntimeError(exec_result);
                        return;
                    }
                    AllowDraw.isError = false;
                    LOG_INFO("Lua script compilation completed");
                }
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

    void LuaGuiMode::RuntimeError(const sol::protected_function_result& result)
    {
        AllowDraw.isError = true;
        if (!result.valid()) {
            sol::error err = result;
            LOG_ERROR("Lua script execution failed: ", err.what());
            current_lua_error = err.what();
        }
        else {
            current_lua_error = "IMGUI internal error, please check the output log";
        }
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