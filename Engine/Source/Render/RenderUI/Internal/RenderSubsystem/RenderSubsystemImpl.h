#pragma once
#include <Tools/Singleton.h>
#include <RenderSubsystem/RenderSubsystem.h>

////////////////////////////
#include <vector>
#include <string>
////////////////////////////

namespace RenderUI {

	struct SubsystemContext
	{
		Subsystem* Subsystem;
		std::string Name;
	};

	class RenderSubsystemImpl : public Singleton<RenderSubsystemImpl> , public ISubsystem
	{
	public:
		virtual void RegisterSubsystem(Context context) override;
		// Control
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void Notification(const char* msg) override;
		// Data
		virtual void* GetSubsystemPublicData(std::string Target, uint8_t Type) override;
	private:
		std::vector<SubsystemContext> Contexts;
	};
}