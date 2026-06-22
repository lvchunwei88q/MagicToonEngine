#pragma once
///////////////////////////////////////
#include <ILog.h>
#include <Object.h>
///////////////////////

#include <RenderSubsystem/RenderSubsystem.h>
#include <RenderMode/ImGuiMode.h>

#include <Tools/GetLanguage.h>

#include <LoggerUI.object.generate.h>

using namespace RenderUI;

namespace RenderEditor {
	MCLASS(MSERIALIZATION);
	class LoggerSwitch : public MObject
	{
		GENERATE_BODY(LoggerSwitch, ENGINE, "EDITORUI");
	public:
		MMEMBER();
		bool showDebug;
		MMEMBER();
		bool showInfo;
		MMEMBER();
		bool showWarning;
		MMEMBER();
		bool showError;
		MMEMBER();
		bool autoScroll;
		MMEMBER();
		bool detailedInformation;
	};

	class LoggerUI final : public RSubsystemTemplate<LoggerUI, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
		std::unique_ptr<LoggerSwitch> Switch;

		std::string Current_Log_Src;
		std::string Current_Log_Info;
		LOG::LogLevel Current_Log_Level;
	};
}