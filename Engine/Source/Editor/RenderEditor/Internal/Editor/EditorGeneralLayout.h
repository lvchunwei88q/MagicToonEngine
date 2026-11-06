#pragma once

#include <Object.h>
#include <EditorGeneralLayout.object.generate.h>

#include <RenderSubsystem/RenderSubsystem.h>
#include <Tools/GetLanguage.h>
#include <Command/ICommandManager.h>

using namespace RenderUI;

namespace RenderEditor {

	enum class EditorGeneralLayoutData : uint8_t
	{
		ViewSwitch
	};

	MCLASS(MSERIALIZATION);
	class ViewSwitch : public MObject
	{
		GENERATE_BODY(ViewSwitch, ENGINE, "EDITORUI");
	public:
		MMEMBER();
		bool DetailsWindow = false;
		MMEMBER();
		bool LuaControl = false;
		MMEMBER();
		bool LoggerWindow = false;
		MMEMBER();
		bool RenderViewWindow = false;
		MMEMBER();
		bool ContentBrowserWindow = false;
		// -----------------------
		MMEMBER();
		bool EditorSettingsView = false;
	};

	class EditorGeneralLayout final : public RSubsystemTemplate<EditorGeneralLayout>
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