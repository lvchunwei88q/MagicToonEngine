#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

using namespace RenderUI;

namespace RenderEditor {
	class RenderView final : public RSubsystemTemplate<RenderView, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
	};
}