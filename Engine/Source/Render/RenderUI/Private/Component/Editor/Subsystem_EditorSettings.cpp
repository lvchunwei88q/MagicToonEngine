#include "EditorUIWarehouse.h"

#include "Component/Editor/EditorSettings.h"
#include "RenderSubsystem/RenderSubsystemImpl.h" // Set RenderUIContext

#include "RenderUI.h"

///////////////////////
// 序列化文件相关
#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>
// 编码转换
#include <Converter.h>
///////////////////////

namespace RenderUI {
    RENDERUI_REGISTER(EditorSettings);

    auto GetLanguageFileName = [](Languages language) {
            std::wstring ContentPath = IO::AbsolutePath::Get().GetContentPath();
            ContentPath += L"\\Config\\Languages\\";

            switch (language)
            {
            case Languages::Chinese:
                ContentPath += L"zh.json";
                break;
            case Languages::English:
                ContentPath += L"en.json";
                break;
            default:LOG_ERROR("There is no corresponding language");
                break;
            }

            return ContentPath;
        };

    void EditorSettings::Init()
    {
        FILE_SERIALIZATION_LOADING(Setting, CONFIG "Editor\\Setting\\", L"EditorSetting.mtdata");

        RenderUIContext& UIContext = RenderSubsystemImpl::Get().SetRenderUIContext();
        std::wstring ContentPath = GetLanguageFileName(Setting.language);

        std::string JsonDump = IO::FileManager::ReadAllText(ContentPath);
        UIContext.EditorLanguage = JSON::parse(JsonDump);
        UIContext.EditorLanguageHash = ComputeStringHash(JsonDump);

        LOG_INFO("Language file loaded! Form: ", IO::Converter::ToNarrowString(ContentPath));
    }

    void EditorSettings::Uninstall()
    {
        FILE_SERIALIZATION_SAVE(Setting, CONFIG "Editor\\Setting\\", L"EditorSetting.mtdata");
    }

    void* EditorSettings::PublicData(uint8_t type)
    {
        return nullptr;
    }

    void EditorSettings::LoadLanguageFile()
    {
        RenderUI::Get().AddTask([this]() {
            RenderUIContext& UIContext = RenderSubsystemImpl::Get().SetRenderUIContext();
            std::wstring ContentPath = GetLanguageFileName(Setting.language);

            std::string JsonDump = IO::FileManager::ReadAllText(ContentPath);
            UIContext.EditorLanguage = JSON::parse(JsonDump);
            UIContext.EditorLanguageHash = ComputeStringHash(JsonDump);
        });
    }

    size_t EditorSettings::ComputeJSONHash(const JSON& data) {
        std::string dump = data.dump();  // 序列化为字符串
        return std::hash<std::string>{}(dump);
    }

    size_t EditorSettings::ComputeStringHash(const std::string& data)
    {
        return std::hash<std::string>{}(data);
    }
}