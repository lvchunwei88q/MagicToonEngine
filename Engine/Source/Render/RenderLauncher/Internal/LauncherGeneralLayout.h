#pragma once
#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderUIWarehouse.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

using namespace RenderUI;

namespace RenderLauncher {
	class LauncherGeneralLayout : public RSubsystemTemplate<LauncherGeneralLayout, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;

		virtual void* PublicData(uint8_t type) override;

		[[nodiscard]] virtual int GetPriority() const override {
			return -100; // High
		}

	private:
		size_t ComputeJSONHash(const JSON& data);
		size_t ComputeStringHash(const std::string& data);

		void ExitProgram();

		bool m_bDragging = false;
		ImVec2 m_dragOffset;
	};
}