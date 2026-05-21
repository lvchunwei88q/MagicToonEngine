#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

using namespace RenderUI;

namespace RenderEditor {
	class RenderView : public RSubsystemTemplate<RenderView, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
	};
}