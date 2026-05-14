#include <RenderUI.h>

#include <RenderSubsystem/RenderSubsystem.h>
#include <mutex>

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
		TaskQueue(); // 先处理任务队列
		GetSubsystem()->Tick();
	}

	void RenderUI::Notification(const char* msg)
	{
		// 处理消息
		GetSubsystem()->Notification(msg);
	}

	void RenderUI::TaskQueue()
	{
		TaskRun = true;
		{
			// 处理任务
			if (TaskNum <= 0) {
				TaskRun = false;
				return;
			}

			// 每个Tick执行一个任务依次处理
			TaskFunction Task = std::move(FunctionQueue.front());
			FunctionQueue.pop();
			TaskNum = FunctionQueue.size();
			Task();
		}

		TaskRun = false;
	}

	void RenderUI::AddTask(TaskFunction Function)
	{
		bool IsComplete = false;
		static std::mutex mtx;

		while (!IsComplete) // 执行到完成
		{
			std::lock_guard<std::mutex> lock(mtx);
			{
				if (!TaskRun.load()) { // 这里主要处理多线程时的任务
					IsComplete = true;
					FunctionQueue.push(std::move(Function));
					TaskNum = FunctionQueue.size();
				}
			}

			if (!IsComplete) {
				std::this_thread::yield();
			}
		}
	}
}