#pragma once
///////////////////////////////////////
#include <ILog.h>
///////////////////////

#include <RenderSubsystem/RenderSubsystem.h>
#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>

namespace RenderUI {
	struct LoggerSwitch {
		bool showDebug;
		bool showInfo;
		bool showWarning;
		bool showError;
		bool autoScroll;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(showDebug, showInfo, showWarning, showError, autoScroll);
		}
	};

	class LoggerUI : public SubsystemTemplate<LoggerUI, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;

	private:
		LoggerSwitch Switch;

		std::string Current_Log_Src;
		std::string Current_Log_Info;
		LOG::LogLevel Current_Log_Level;
	};
}