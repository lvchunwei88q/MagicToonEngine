#pragma once

#include "Common/RENDERUI_API.h"
#include <sol/sol.hpp> // lua bind
#include <Common/Check.h>

namespace RenderUI {
	class LuaGuiMode;

	struct DrawCancel
	{
		std::atomic<bool> isError = false;
		std::atomic<bool> isCompileError = false;
		std::atomic<bool> isLoading{ false };

		bool Allow() const {
			return isError || isCompileError || isLoading;
		}
	};

	struct LuaMemberContext {
		LuaGuiMode* member;
		std::string member_name;
	};

	struct LuaMember {
		LuaGuiMode* member;
		std::string member_name;
		LuaMember() = default;
		LuaMember(LuaGuiMode* m, std::string n)
			: member(m), member_name(n)
		{
		}
	};

	DISABLE_DLL_WARNINGS_PUSH;
	class RENDERUI_API LuaGuiMode
	{
	public:
		void Draw();
		virtual void Preprocessing() {};
		void RegisterLua(std::string lua_type); // 注册Lua每次会清除状态
		void UpdateLua();

		void RuntimeError();
		void RuntimeError(const sol::protected_function_result& result);

		const std::string& GetLuaType() { return current_lua_type; }
		const bool GetisLoading();
	protected:
		DrawCancel AllowDraw;

		std::string lua_path;
		std::string current_lua_type = "null";// lua_type 就是Class加载lua时选择的lua文件
		std::string current_lua_error = "noError";

		sol::state lua; // state
		sol::protected_function lua_script; // current lua script
	};
	DISABLE_DLL_WARNINGS_POP;

	class RENDERUI_API ILuaUIMember
	{
	public:
		virtual void Register(LuaMemberContext member) = 0;
		virtual const std::vector<LuaMember>& GetLuaMember() = 0;
	};

	RENDERUI_API ILuaUIMember* GetLuaUIMember();

	// TODO 更加智能化控制
#define REGISTER_LUA_MEMBER(Object,Name)			\
		GetLuaUIMember()->Register({Object,Name});
}