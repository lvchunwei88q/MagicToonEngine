#pragma once
#include <Tools/Singleton.h>
#include <Subsystem/SubsystemTemplate.h>
#include <sol/sol.hpp> // lua bind

#include <EditorUIComponentSwitch.h>
#include <vector>

namespace RenderUI {
	class LuaUIControl; // lua_type 就是Class加载lua时选择的lua文件

	struct DrawCancel
	{
		std::atomic<bool> isError = false;
		std::atomic<bool> isCompileError = false;
		std::atomic<bool> isLoading{ false };

		bool is() const {
			return isError || isCompileError || isLoading;
		}
	};

	struct LuaMember {
		LuaUIControl* member;
		std::string member_name;
		std::string lua_type;
		LuaMember() = default;
		LuaMember(LuaUIControl* m, std::string n);
	};

	class LuaUIControl
	{
	public:
		void Draw();
		virtual void Preprocessing() {};
		void LoadLua(std::string lua_type);
		void UpdateLua();

		const std::string GetLuaType() { return current_lua_type; }
		const bool GetisLoading();
	protected:
		DrawCancel isDraw;

		std::string lua_path;
		std::string current_lua_type = "null";
		sol::state lua; // state
		sol::protected_function lua_script; // current lua script
	};

	class LuaUIMember : public Singleton<LuaUIMember>
	{
	public:
		void Register(LuaMember member);
		const std::vector<LuaMember>& GetLuaMember();

	private:
		std::vector<LuaMember> members;
	};

	#define REGISTER_LUA_MEMBER(Object,Name)			\
		LuaUIMember::Get().Register({Object,Name});

	class LuaUIControlWindows : public LuaUIControl, public SubsystemTemplate<LuaUIControlWindows, Core::SubsystemContext::Priority::Low>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();

		virtual void Preprocessing() override;
	};
}