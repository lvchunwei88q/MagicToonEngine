// Command/FileBrowserCommands.h
#pragma once
#include <Command/ICommand.h>
#include <Tools/Singleton.h>
#include <string>
#include <RenderSubsystem/RenderSubsystem.h>

using namespace RenderUI;
using namespace Command;
namespace RenderEditor {
    
    enum class Languages {
        English,
        Chinese
    };

    struct EditorSetting : public Singleton<EditorSetting> {
        // Languages
        Languages language = Languages::English;

        template<class Archive>
        void serialize(Archive& archive) {
            archive(language);
        }
    };

    // ==================== 切换语言命令 ====================
    class SwitchLanguageCommand final : public ICommand {
    private:
        Languages m_oldLanguage;
        Languages m_newLanguage;
        std::string m_description;

    public:
        SwitchLanguageCommand(Languages newLanguage)
            : m_newLanguage(newLanguage) {
            // 获取当前语言（假设有一个全局设置管理器）
            m_oldLanguage = EditorSetting::Get().language;

            m_description = std::string("Editor Setting Switch language ");
        }

        void Execute() override {
            // 应用新语言
            EditorSetting::Get().language = (m_newLanguage);
			GetSubsystem()->CallSubsystemFunction("EditorSettings","LoadLanguageFile", nullptr); // 切换语言后需要重新加载字体
        }

        void Undo() override {
            // 恢复旧语言
            EditorSetting::Get().language = (m_oldLanguage);
            GetSubsystem()->CallSubsystemFunction("EditorSettings", "LoadLanguageFile", nullptr); // 切换语言后需要重新加载字体
        }

        std::string GetDescription() const override {
            return m_description;
        }

        bool CanMergeWith(const ICommand* other) const override {
            // 语言切换不需要合并（每次切换都是独立操作）
            return false;
        }
    };
} // namespace RenderEditor