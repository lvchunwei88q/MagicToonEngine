#include <Component/DetailsWindow.h>
#include <MagicUIContext.h>
#include <ILog.h>
//////////////////////////////
#include <AbsolutePath.h>
#include <Converter.h>
//////////////////////////////

namespace RenderUI {
    AUTO_REGISTER(DetailsPanel);

    bool DetailsPanel::Init()
    {
        MteGUIContext::Get().BindLuaFunction(lua);
        LoadLuaFile(DetailsPanelType::JSON);
        return true;
    }

    void DetailsPanel::Uninstall()
    {
    }

    void DetailsPanel::LoadLuaFile(DetailsPanelType Type)
    {
        std::wstring script_path = IO::AbsolutePath::Get().GetScriptPath();
        std::string Path = IO::Converter::ToNarrowString(script_path) + "\\EditorUI\\JsonPanel.lua";
        sol::load_result result = lua.load_file(Path);
        if (!result.valid()) {
            sol::error err = result;
            LOG_ERROR("Lua script compilation failed: ", err.what());
            return;
        }
        lua_script = result;

        // 执行一次Lua
        lua_script();
        Lua_Error = false;
        LOG_INFO("Lua script compilation completed");
    }

    void DetailsPanel::Draw()
    {
        // test lua
        if (!Lua_Error) {
            lua_getglobal(lua, "Draw");
            int ret = lua_pcall(lua, 0, 0, 0);
            if (ret != LUA_OK) {
                // 错误信息会在栈顶
                const char* err = lua_tostring(lua, -1);
                LOG_ERROR("Lua Draw() Error:", err);
                Lua_Error = true;
                lua_pop(lua, 1);
            }
        }
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