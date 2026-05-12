#include <RenderSubsystem/RenderSubsystemImpl.h>
#include <EditorUIWarehouse.h>
#include <ILog.h>

#include <algorithm>

namespace RenderUI {
	ISubsystem* GetSubsystem()
	{
		return &RenderSubsystemImpl::Get();
	}

	void RenderSubsystemImpl::RegisterSubsystem(Context context)
	{
		SubsystemContext sub_context;
		sub_context.Name = context.Name;
		sub_context.Subsystem = context.Subsystem;
		Contexts.push_back(std::move(sub_context));
	}

	void RenderSubsystemImpl::Init()
	{
		std::sort(Contexts.begin(), Contexts.end(),
		[](const SubsystemContext& a, const SubsystemContext& b) {
			return a.Subsystem->GetPriority() < b.Subsystem->GetPriority();
		});

		for (size_t i = 0; i < Contexts.size(); i++)
		{
			auto& context = Contexts[i];
			if (context.Subsystem)
				context.Subsystem->Init();
		}
	}

	void RenderSubsystemImpl::Uninstall()
	{
		for (size_t i = Contexts.size(); i-- > 0;)
		{
			auto& context = Contexts[i];
			if (context.Subsystem)
				context.Subsystem->Uninstall();
		}
	}

	void RenderSubsystemImpl::Tick()
	{
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			auto& context = Contexts[i];
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
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			auto& context = Contexts[i];
			if (context.Name.find(Target) != std::string::npos) {
				return context.Subsystem->PublicData(Type);
			}
		}
		return nullptr;
	}

	void RenderSubsystemImpl::Notification(const char* msg)
	{
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			auto& context = Contexts[i];
			if (context.Subsystem)
				context.Subsystem->Notification(msg);
		}
	}
}
