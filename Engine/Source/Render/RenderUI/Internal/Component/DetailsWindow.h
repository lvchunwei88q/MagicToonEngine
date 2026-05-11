#pragma once
#include <Subsystem/SubsystemTemplate.h>
#include <GuiComponentSwitch.h>
#include <sol/sol.hpp> // lua bind

namespace RenderUI {

	enum class DetailsPanelType {
		JSON,
	};

	class DetailsPanel : public SubsystemTemplate<DetailsPanel,Core::SubsystemContext::Priority::Low>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();
		void LoadLuaFile(DetailsPanelType Type);

		void Draw();
	private:
		sol::state lua; // state
		sol::protected_function lua_script;

		bool Lua_Error = false;
	};
}