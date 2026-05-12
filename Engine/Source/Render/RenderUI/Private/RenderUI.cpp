#include <RenderUI.h>
#include <RenderSubsystem/RenderSubsystem.h>

namespace RenderUI {
	AUTO_REGISTER_SINGLETON_NOTIFICATION(RenderUI,"IMGUI", Normal);

	bool RenderUI::Init()
	{
		GetSubsystem()->Init(); // render subsystem
		return true;
	}

	void RenderUI::Uninstall()
	{
		GetSubsystem()->Uninstall(); // render subsystem
	}

	void RenderUI::Tick()
	{
		// tick function
		GetSubsystem()->Tick();
	}

	void RenderUI::Notification(const char* msg)
	{
		// 处理消息
		GetSubsystem()->Notification(msg);
	}
}