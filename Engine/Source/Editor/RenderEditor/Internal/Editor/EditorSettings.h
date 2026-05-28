#pragma once
#include <RenderSubsystem/RenderSubsystem.h>

#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>
#include <Commands/EditorSettingCommands.h>
#include <Command/ICommandManager.h>

using namespace RenderUI;

namespace RenderEditor {

	class EditorSettings final : public RSubsystemTemplate<EditorSettings, ModeType::ImGui>, public ImGuiMode
	{
	public:
		virtual void Init() override;
		virtual void Uninstall() override;
		virtual void Tick() override;
		virtual void* PublicData(uint8_t type) override;


		RS_REGISTER_METHODS(
			RS_METHOD(LoadLanguageFile)
		);
	private:
		// 分隔符
		float ModulesWidth = 150.0f;
		const float splitterWidth = 4.0f;

		EditorSetting& Setting = EditorSetting::Get();

		void OnUserSelectLanguage(Languages newLang) {
			auto cmd = std::make_unique<SwitchLanguageCommand>(newLang);
			// 提交给 CommandManager 执行
			GetCommandManager()->ExecuteCommand(std::move(cmd));
		}

		void LoadLanguageFile(void*);

		size_t ComputeJSONHash(const JSON& data);
		size_t ComputeStringHash(const std::string& data);
	};
}