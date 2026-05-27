#pragma once

#include "Common/RENDERUI_API.h"
#include "RenderMode/RenderMode.h" // 绘制模式 
#include <imgui.h>
#include <Common/Compiler.h>

#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <Tools/Singleton.h>
#include <functional>

/*
* 控制UI的绘制，具体来说我们将UI理解为组件的形式，并且划分俩个不同的UI 原生IMGUI 与 LUAGUI，
* 在注册时就可以选择，并且我们使用接口模式提供了一个模块接口
* 并且 RenderUI 的Subsystem初始化时机是 Normal 所以必须要在 Normal 初始化之前设置windows句柄与渲染API设备
*/
namespace RenderUI {
	BEGIN_PIMPL;

	using TaskFunction = std::function<void()>;

	class RENDERUI_API RenderUIManager : public Core::Subsystem , public Singleton<RenderUIManager>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();
		virtual void Notification(Core::NotificationContext Context);

		// tick
		void Tick();
		
		// Context
		ImGuiContext* GetImGuiContext();
		
		/*
		* 在Tick之前的任务队列，用于向RenderUIManager设置状态时使用，由于在Tick之前设置状态不会被使用
		* 所以使用TaskQueue就可以避免多线程之间的资源竞争
		*/
		void TaskQueue();
		void AddTask(TaskFunction Function);

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;

		// dpi scale
		float main_scale = 1.0f;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(RenderUIManager);

	END_PIMPL;
}