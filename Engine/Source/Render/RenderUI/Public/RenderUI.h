#pragma once

#include <Common/RENDERUI_API.h>
#include <RenderMode/RenderMode.h> // 绘制模式 

#include <atomic>
#include <queue>

#include <Subsystem/Subsystem.h>
#include <Subsystem/SubsystemTemplate.h>
#include <Tools/Singleton.h>
#include <functional>

/*
* 控制UI的绘制，具体来说我们将UI理解为组件的形式，并且划分俩个不同的UI 原生IMGUI 与 LUAGUI，
* 在注册时就可以选择，并且我们使用接口模式提供了一个模块接口
* 
*/
namespace RenderUI {
	using TaskFunction = std::function<void()>;

	class RENDERUI_API RenderUI : public Core::Subsystem , public Singleton<RenderUI>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();
		virtual void Notification(const char* msg);

		// tick
		void Tick();

		/*
		* 在Tick之前的任务队列，用于向RenderUI设置状态时使用，由于在Tick之前设置状态不会被使用
		* 所以使用TaskQueue就可以避免多线程之间的资源竞争
		*/
		void TaskQueue();
		void AddTask(TaskFunction Function);

	private:
		// Task var
		std::atomic<bool> TaskRun = false;
		std::atomic<int> TaskNum = 0;
		std::queue<TaskFunction> FunctionQueue;
	};

	AUTO_REGISTER_SINGLETON_INCLUDE(RenderUI);
}