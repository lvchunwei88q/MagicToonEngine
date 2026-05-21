#pragma once
#include <Tools/Singleton.h>
#include "RenderSubsystem/RenderSubsystem.h"

////////////////////////////
#include <vector>
#include <string>
////////////////////////////

namespace RenderUI {

	struct SubsystemContext
	{
		RSubsystem* Subsystem;
		std::string Name;
	};

	class RenderSubsystemImpl : public Singleton<RenderSubsystemImpl> , public IRSubsystem
	{
	public:
		virtual void RegisterSubsystem(RegisterSubsystemContext context) override;
		// Control
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void Notification(const char* msg) override;
		// Data
		virtual void* GetSubsystemPublicData(std::string Target, uint8_t Type) override;
		virtual [[nodiscard]] const RenderUIContext& GetRenderUIContext() const override;
		virtual [[nodiscard]] RenderUIContext& SetRenderUIContext() override;
	private:
		std::vector<SubsystemContext> Subsystems;

		RenderUIContext UIContext;
	};
}