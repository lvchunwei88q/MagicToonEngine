#pragma once
#include <RenderSubsystem/RenderSubsystem.h>

#include <cereal/cereal.hpp>
#include <RenderMode/ImGuiMode.h>
#include <Tools/GetLanguage.h>

using namespace RenderUI;

namespace RenderEditor {
	enum class Languages {
		English,
		Chinese
	};

	struct EditorSetting {
		// Languages
		Languages language = Languages::English;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(language);
		}
	};

	class EditorSettings : public RSubsystemTemplate<EditorSettings, ModeType::ImGui>, public ImGuiMode
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

		EditorSetting Setting;

		void LoadLanguageFile();

		size_t ComputeJSONHash(const JSON& data);
		size_t ComputeStringHash(const std::string& data);
	};
}