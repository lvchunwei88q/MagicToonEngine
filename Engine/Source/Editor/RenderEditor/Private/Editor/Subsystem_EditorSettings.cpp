#include <RenderUIWarehouse.h>

#include <RenderUI.h>
#include "Editor/EditorSettings.h"
#include <RenderSubsystem/RenderSubsystem.h> // Set RenderUIContext

///////////////////////
// 序列化文件相关
#include <IO.h> 
#include <fstream>
#include <Tools/SerializeMacro.h>

// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制
///////////////////////

namespace RenderEditor {
    RENDERUI_REGISTER(EditorSettings);

    auto GetLanguageFileName = [](Languages language) {
            std::wstring ContentPath = IO::AbsolutePath::Get().GetContentDirectory().GetConfig();
            ContentPath += L"Languages\\";

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

        RenderUIContext& UIContext = GetSubsystem()->SetRenderUIContext();
        std::wstring ContentPath = GetLanguageFileName(Setting.language);

        std::string JsonDump = IO::ReadAllText(ContentPath);
        UIContext.EditorLanguage = JSON::parse(JsonDump);
        UIContext.EditorLanguageHash = ComputeStringHash(JsonDump);

        LOG_INFO("Language file loaded! Form: ", IO::ToNarrowString(ContentPath));
    }

    void EditorSettings::Uninstall()
    {
        FILE_SERIALIZATION_SAVE(Setting, CONFIG "Editor\\Setting\\", L"EditorSetting.mtdata");
    }

    void* EditorSettings::PublicData(uint8_t type)
    {
        return nullptr;
    }

    void EditorSettings::LoadLanguageFile(void*)
    {
        RenderUIManager::Get().AddTask([this]() {
            RenderUIContext& UIContext = GetSubsystem()->SetRenderUIContext();
            std::wstring ContentPath = GetLanguageFileName(Setting.language);

            std::string JsonDump = IO::ReadAllText(ContentPath);
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