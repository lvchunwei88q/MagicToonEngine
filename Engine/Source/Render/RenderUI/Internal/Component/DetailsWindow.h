#pragma once
#include <Subsystem/SubsystemTemplate.h>
#include <EditorUIComponentSwitch.h>

#include <Component/LuaUIControl.h>

namespace RenderUI {

	enum class DetailsPanelType {
		JSON,
	};

	class DetailsPanel : public SubsystemTemplate<DetailsPanel,Core::SubsystemContext::Priority::Low> , public LuaUIControl
	{
	public:
		virtual bool Init();
		virtual void Uninstall();

	private:
	};
}