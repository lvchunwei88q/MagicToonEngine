#include "RenderSubsystem/RenderSubsystemImpl.h"
#include "RenderUIWarehouse.h"
#include <ILog.h>

#include <algorithm>

namespace RenderUI {
	IRSubsystem* GetSubsystem()
	{
		return &RenderSubsystemImpl::Get();
	}

	namespace {
		bool FunctionExists(const std::vector<std::string>& names, std::string funcName) {
			for (const auto& name : names) {
				if (strcmp(name.c_str(), funcName.c_str()) == 0) {
					return true;
				}
			}
			return false;
		}
	}

	void RenderSubsystemImpl::RegisterSubsystem(RegisterSubsystemContext context)
	{
		SubsystemContext sub_context;
		sub_context.Name = context.Name;
		sub_context.Subsystem = context.Subsystem;

		if (sub_context.Subsystem->Array) { //	如果子系统提供了函数数组，则将其注册到上下文中
			for (size_t i = 0; i < sub_context.Subsystem->Array->FunctionArraySize; i++)
			{
				if (FunctionExists(sub_context.FunctionNames, sub_context.Subsystem->Array->FunctionNames[i])) {
					// 函数已存在，打印日志
					LOG_WARNING("Function ", sub_context.Subsystem->Array->FunctionNames[i]," already registered, skipping duplicate");
					continue;
				}
				sub_context.FunctionNames.push_back(sub_context.Subsystem->Array->FunctionNames[i]);
				sub_context.FunctionArray.push_back(sub_context.Subsystem->Array->FunctionArray[i]);
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
			if (!context.FunctionArray.empty() && !context.FunctionNames.empty() && context.Name == Target) {
				for (size_t j = 0; j < context.FunctionNames.size(); j++) {
					if (context.FunctionNames[j] == Func) {
						context.FunctionArray[j](data);
						break;		// 找到函数后执行并跳出循环 同时应该保证函数名称的唯一性
					}
				}
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
