#include <Component/Editor/EditorSettings.h>

namespace RenderUI {
    RENDERUI_REGISTER(EditorSettings);

    void EditorSettings::Init()
    {
    }

    void EditorSettings::Uninstall()
    {
    }

    void* EditorSettings::PublicData(uint8_t type)
    {
        return nullptr;
    }
}