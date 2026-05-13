#pragma once
#include <RenderSubsystem/RenderSubsystem.h>

#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>

namespace RenderUI {
	class EditorSettings : public SubsystemTemplate<EditorSettings, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
		// 分隔符
		float ModulesWidth = 150.0f;
		const float splitterWidth = 4.0f;
	};
}