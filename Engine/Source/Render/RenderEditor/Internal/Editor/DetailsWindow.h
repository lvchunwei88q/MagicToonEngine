#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderMode/LuaGuiMode.h>
#include <Tools/GetLanguage.h>

#define WINDOWS_NAME "Details Panel"

using namespace RenderUI;

namespace RenderEditor {

	enum class DetailsPanelType {
		JSON,
	};

	class DetailsPanel : public RSubsystemTemplate<DetailsPanel, ModeType::LuaGui> , public LuaGuiMode
	{
	public:
		virtual void Init();
		virtual void Uninstall();
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;
		virtual void Notification(const char* msg);

	private:
	};
}