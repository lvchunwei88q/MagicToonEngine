#pragma once

#include <RenderSubsystem/RenderSubsystem.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>
#include <Command/ICommandManager.h>

using namespace RenderUI;

namespace RenderEditor {

	enum class EditorGeneralLayoutData : uint8_t
	{
		ViewSwitch
	};

	struct ViewSwitch {
		bool DetailsWindow;
		bool LuaControl;
		bool LoggerWindow;
		bool RenderViewWindow;
		bool ContentBrowserWindow;
		// -----------------------
		bool EditorSettingsView;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(DetailsWindow, LuaControl, LoggerWindow,
				RenderViewWindow, ContentBrowserWindow, EditorSettingsView);
		}
	};

	class EditorGeneralLayout final : public RSubsystemTemplate<EditorGeneralLayout, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;

		virtual void* PublicData(uint8_t type) override;

		[[nodiscard]] virtual int GetPriority() const override {
			return -100; // High
		}

	protected:
		void ExitProgram();
	private:
		ViewSwitch Switch;
	};
}