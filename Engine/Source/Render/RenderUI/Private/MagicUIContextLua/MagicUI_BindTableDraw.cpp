#include <MagicUIContext.h>

namespace RenderUI {

    void MteGUIContext::BindTableDraw(sol::state& lua, const std::string& name) {
        sol::table gui_type = lua[name];
        // ========== Tables ==========
        gui_type.set("BeginTable", &MteGUIContext::BeginTable);
        gui_type.set("EndTable", &MteGUIContext::EndTable);
        gui_type.set("TableSetupColumn", &MteGUIContext::TableSetupColumn);
        gui_type.set("TableHeadersRow", &MteGUIContext::TableHeadersRow);
        gui_type.set("TableNextRow", &MteGUIContext::TableNextRow);
        gui_type.set("TableSetColumnIndex", &MteGUIContext::TableSetColumnIndex);
        gui_type.set("TableNextColumn", &MteGUIContext::TableNextColumn);

        // ========== Images ==========
        gui_type.set("Image", &MteGUIContext::Image);
        gui_type.set("ImageButton", &MteGUIContext::ImageButton);

        // ========== Draw List Primitives ==========
        gui_type.set("DrawRect", &MteGUIContext::DrawRect);
        gui_type.set("DrawFilledRect", &MteGUIContext::DrawFilledRect);
        gui_type.set("DrawFilledRectRotated", &MteGUIContext::DrawFilledRectRotated);
        gui_type.set("DrawLine", &MteGUIContext::DrawLine);
        gui_type.set("DrawCircle", &MteGUIContext::DrawCircle);
        gui_type.set("DrawFilledCircle", &MteGUIContext::DrawFilledCircle);
        gui_type.set("DrawImageRect", &MteGUIContext::DrawImageRect);
        gui_type.set("DrawTextDefault", &MteGUIContext::DrawTextDefault);
        gui_type.set("DrawTextAligned", &MteGUIContext::DrawTextAligned);
        gui_type.set("DrawTextRotated90Aligned", &MteGUIContext::DrawTextRotated90Aligned);
        gui_type.set("DrawTextExAligned", &MteGUIContext::DrawTextExAligned);

        gui_type.set("CalcTextSizeA", [](MteGUIContext& self, const std::string& text, float fontSize,
            const std::string& fontPath, float lineHeight, float letterSpacing) -> std::pair<float, float> {
                return self.CalcTextSizeA(text, fontSize, fontPath, lineHeight, letterSpacing);
            });
        gui_type.set("CalcTextSizeWrappedA", [](MteGUIContext& self, const std::string& text, float fontSize, float wrapWidth,
            const std::string& fontPath, float lineHeight, float letterSpacing) -> std::pair<float, float> {
                return self.CalcTextSizeWrappedA(text, fontSize, wrapWidth, fontPath, lineHeight, letterSpacing);
            });

        gui_type.set("PushDrawListClipRect", &MteGUIContext::PushDrawListClipRect);
        gui_type.set("PopDrawListClipRect", &MteGUIContext::PopDrawListClipRect);
    }

} // namespace RenderUI