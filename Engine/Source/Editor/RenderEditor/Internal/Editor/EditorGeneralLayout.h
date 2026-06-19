#pragma once

#include <Object.h>
#include <EditorGeneralLayout.object.generate.h>

#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>
#include <Command/ICommandManager.h>

using namespace RenderUI;

namespace RenderEditor {

	enum class EditorGeneralLayoutData : uint8_t
	{
		ViewSwitch
	};

	MCLASS(MSERIALIZATION);
	class ViewSwitch : public Core::Object
	{
		GENERATE_BODY(ViewSwitch, ENGINE, "EDITORUI");
	public:
		MMEMBER();
		bool DetailsWindow;
		MMEMBER();
		bool LuaControl;
		MMEMBER();
		bool LoggerWindow;
		MMEMBER();
		bool RenderViewWindow;
		MMEMBER();
		bool ContentBrowserWindow;
		// -----------------------
		MMEMBER();
		bool EditorSettingsView;
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
		std::unique_ptr<ViewSwitch> Switch;
	};
}