#pragma once
#include <Tools/Singleton.h>
#include "RenderMode/LuaGuiMode.h"
#include "RenderSubsystem/RenderSubsystem.h"

#include <vector>
#include "RenderMode/LuaGuiModeInternal.h" // 内部使用
#include "Tools/GetLanguage.h"

#define WINDOWS_NAME "LuaControl Panel"

namespace RenderUI {
	class LuaUIControlWindows : public LuaGuiMode, public SubsystemTemplate<LuaUIControlWindows, ModeType::LuaGui>
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