#pragma once

#include <Common/RENDERUI_API.h>
#include <RenderMode/RenderMode.h> // 绘制模式 

#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <Tools/Singleton.h>

/*
* 控制UI的绘制，具体来说我们将UI理解为组件的形式，并且划分俩个不同的UI 原生IMGUI 与 LUAGUI，
* 在注册时就可以选择，并且我们使用接口模式提供了一个模块接口
* 
*/
namespace RenderUI {
	class RENDERUI_API RenderUI : public Core::Subsystem , public Singleton<RenderUI>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();
		virtual void Notification(const char* msg);

		// tick
		void Tick();
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(RenderUI);
}