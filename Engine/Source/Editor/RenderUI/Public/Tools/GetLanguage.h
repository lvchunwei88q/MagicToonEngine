#pragma once
#include "RenderSubsystem/RenderSubsystem.h"
#include "Common/RENDERUI_API.h"

#include <Common/Check.h>
#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

namespace RenderUI {
    DISABLE_DLL_WARNINGS_PUSH;

    namespace Language {
        class RENDERUI_API LanguageManager {
        private:
            static JSON m_langData;
            static size_t s_languageHash;
            static std::unordered_map<std::string, std::string> m_cache;

        public:
            static std::string Get(const std::string& src);
            static void ClearCache();

        private:
            // 设置 JSON 数据并清除缓存
            static void SetLanguageData(const JSON& data);
        };
    }

    using Lang = Language::LanguageManager;

    inline std::string GetLanguageFast(const std::string& src, const JSON& langData) {
        const JSON* current = &langData;
        size_t start = 0;
        size_t end = 0;

        while (end != std::string::npos) {
            end = src.find('.', start);
            std::string_view key = std::string_view(src).substr(start, end - start);

            if (!current->is_object() || !current->contains(key)) {
                return src;
            }
            current = &(*current)[key];
            start = end + 1;
        }

        return current->is_string() ? current->get<std::string>() : src;
    }

    DISABLE_DLL_WARNINGS_POP;
}