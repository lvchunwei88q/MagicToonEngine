#pragma once
#include <Tools/Singleton.h>
#include <RenderMode/LuaGuiMode.h>
#include <RenderSubsystem/RenderSubsystem.h>

#include <vector>
#include <RenderMode/LuaGuiMode.h>
#include <Tools/GetLanguage.h>

#define WINDOWS_NAME "LuaControl Panel"

using namespace RenderUI;

namespace RenderEditor {
	class LuaUIControlWindows final : public LuaGuiMode, public RSubsystemTemplate<LuaUIControlWindows, ModeType::LuaGui>
	{
	public:
		virtual void Init()override;
		virtual void Uninstall()override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;
		virtual void Notification(const char* msg) override;

		virtual void Preprocessing() override;
	};
}