#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>

namespace RenderUI {
	class RenderView : public SubsystemTemplate<RenderView, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
	};
}