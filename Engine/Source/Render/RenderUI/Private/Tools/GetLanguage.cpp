#include "Tools/GetLanguage.h"

namespace RenderUI {
    namespace Language {
        JSON LanguageManager::m_langData;
        size_t LanguageManager::s_languageHash = 0;
        std::unordered_map<std::string, std::string> LanguageManager::m_cache;

        std::string LanguageManager::Get(const std::string& src) {
            const RenderUIContext& UIContext = GetSubsystem()->GetRenderUIContext();
            if (s_languageHash != UIContext.EditorLanguageHash)
            {
                SetLanguageData(UIContext.EditorLanguage);
                s_languageHash = UIContext.EditorLanguageHash;
            }

            // Check 
            auto it = m_cache.find(src);
            if (it != m_cache.end()) {
                return it->second;
            }

            const JSON* current = &m_langData;
            size_t start = 0;
            size_t end = 0;

            while (end != std::string::npos) {
                end = src.find('.', start);
                std::string key = src.substr(start, end - start);

                if (!current->is_object() || !current->contains(key)) {
                    m_cache[src] = src;
                    return src;
                }
                current = &(*current)[key];
                start = end + 1;
            }

            std::string result = current->is_string() ? current->get<std::string>() : src;
            m_cache[src] = result;
            return result;
        }

        void LanguageManager::ClearCache() {
            m_cache.clear();
        }

        void LanguageManager::SetLanguageData(const JSON& data) {
            m_langData = data;
            m_cache.clear();
        }
	}
}