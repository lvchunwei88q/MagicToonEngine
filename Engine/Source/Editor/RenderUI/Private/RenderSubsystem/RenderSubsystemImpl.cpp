#include "RenderSubsystem/RenderSubsystemImpl.h"
#include "RenderUIWarehouse.h"
#include <ILog.h>

#include <algorithm>

namespace RenderUI {
	IRSubsystem* GetSubsystem()
	{
		return &RenderSubsystemImpl::Get();
	}

	void RenderSubsystemImpl::RegisterSubsystem(RegisterSubsystemContext context)
	{
		SubsystemContext sub_context;
		sub_context.Name = context.Name;
		sub_context.Subsystem = context.Subsystem;

		if (sub_context.Subsystem->Array) { //	如果子系统提供了函数数组，则将其注册到上下文中
			for (size_t i = 0; i < sub_context.Subsystem->Array->FunctionArraySize; i++)
			{
				sub_context.Functions[sub_context.Subsystem->Array->FunctionNames[i]]
					= sub_context.Subsystem->Array->FunctionArray[i];
			}
			delete sub_context.Subsystem->Array;
			sub_context.Subsystem->Array = nullptr;
		}

		Subsystems.push_back(std::move(sub_context));
	}

	void RenderSubsystemImpl::Init()
	{
		std::sort(Subsystems.begin(), Subsystems.end(),
		[](const SubsystemContext& a, const SubsystemContext& b) {
			return a.Subsystem->GetPriority() < b.Subsystem->GetPriority();
		});

		for (size_t i = 0; i < Subsystems.size(); i++)
		{
			auto& context = Subsystems[i];
			if (context.Subsystem)
				context.Subsystem->Init();
		}
	}

	void RenderSubsystemImpl::Uninstall()
	{
		for (size_t i = Subsystems.size(); i-- > 0;)
		{
			auto& context = Subsystems[i];
			if (context.Subsystem)
				context.Subsystem->Uninstall();
		}
	}

	void RenderSubsystemImpl::Tick()
	{
		for (size_t i = 0; i < Subsystems.size(); i++)
		{
			auto& context = Subsystems[i];
			if (context.Subsystem) {
				switch (context.Subsystem->GetModeType())
				{
				case ModeType::LuaGui: {
					context.Subsystem->Tick();
				}
					break;
				case ModeType::ImGui: {
					context.Subsystem->Tick();
				}
					break;
				default:LOG_ERROR("No specified rendering mode");
					break;
				}
			}
		}
	}

	void* RenderSubsystemImpl::GetSubsystemPublicData(std::string Target, uint8_t Type)
	{
		for (size_t i = 0; i < Subsystems.size(); i++)
		{
			auto& context = Subsystems[i];
			if (context.Name == Target) {
				return context.Subsystem->PublicData(Type);
			}
		}
		return nullptr;
	}

	void RenderSubsystemImpl::CallSubsystemFunction(std::string Target, std::string Func,void* data)
	{
		for (size_t i = 0; i < Subsystems.size(); i++)
		{
			auto& context = Subsystems[i];
			if (!context.Functions.empty() && context.Name == Target) {
				if (context.Functions.find(Func) != context.Functions.end())
					context.Functions[Func](nullptr);
				break;
			}
		}
	}

	const RenderUIContext& RenderSubsystemImpl::GetRenderUIContext() const
	{
		return UIContext;
	}

	RenderUIContext& RenderSubsystemImpl::SetRenderUIContext()
	{
		return UIContext;
	}

	void RenderSubsystemImpl::Notification(const char* msg)
	{
		for (size_t i = 0; i < Subsystems.size(); i++)
		{
			auto& context = Subsystems[i];
			if (context.Subsystem)
				context.Subsystem->Notification(msg);
		}
	}
}
