#include <MagicUIContext.h>

namespace RenderUI {

    void MteGUIContext::BindLayout(sol::state& lua, const std::string& name) {
        sol::table gui_type = lua[name];;
        // ========== Layout Helpers ==========
        gui_type.set("BeginGroup", &MteGUIContext::BeginGroup);
        gui_type.set("EndGroup", &MteGUIContext::EndGroup);
        gui_type.set("SameLine", &MteGUIContext::SameLine);
        gui_type.set("AlignTextToFramePadding", &MteGUIContext::AlignTextToFramePadding);
        gui_type.set("Separator", &MteGUIContext::Separator);
        gui_type.set("Spacing", &MteGUIContext::Spacing);
        gui_type.set("Dummy", &MteGUIContext::Dummy);
        gui_type.set("NewLine", &MteGUIContext::NewLine);
        gui_type.set("Indent", [](MteGUIContext& self, float w) { ImGui::Indent(w); });
        gui_type.set("Unindent", [](MteGUIContext& self, float w) { ImGui::Unindent(w); });

        // ========== Child Regions ==========
        gui_type.set("BeginChild", sol::overload(
            [](MteGUIContext& self, const std::string& name) -> bool {
                return self.BeginChild(name, 0.0f, 0.0f, false);
            },
            [](MteGUIContext& self, const std::string& name, float width, float height, bool border) -> bool {
                return self.BeginChild(name, width, height, border);
            },
            [](MteGUIContext& self, const std::string& name, float width, float height, ImGuiChildFlags childFlags, ImGuiWindowFlags windowFlags) -> bool {
                return ImGui::BeginChild(name.c_str(), ImVec2(width, height), childFlags, windowFlags);
            }
        ));
        gui_type.set("EndChild", &MteGUIContext::EndChild);

        // ========== Scrolling ==========
        gui_type.set("SetScrollHereY", &MteGUIContext::SetScrollHereY);
        gui_type.set("GetScrollY", &MteGUIContext::GetScrollY);
        gui_type.set("GetScrollMaxY", &MteGUIContext::GetScrollMaxY);

        // ========== Layout Query ==========
        gui_type.set("CalcTextWidth", &MteGUIContext::CalcTextWidth);
        gui_type.set("GetContentRegionAvailWidth", &MteGUIContext::GetContentRegionAvailWidth);
        gui_type.set("GetContentRegionAvailHeight", &MteGUIContext::GetContentRegionAvailHeight);
        gui_type.set("GetCursorPosX", &MteGUIContext::GetCursorPosX);
        gui_type.set("GetCursorPosY", &MteGUIContext::GetCursorPosY);
        gui_type.set("SetCursorPosX", &MteGUIContext::SetCursorPosX);
        gui_type.set("SetCursorPosY", &MteGUIContext::SetCursorPosY);
        gui_type.set("GetWindowPosX", &MteGUIContext::GetWindowPosX);
        gui_type.set("GetWindowPosY", &MteGUIContext::GetWindowPosY);
        gui_type.set("GetWindowWidth", &MteGUIContext::GetWindowWidth);
        gui_type.set("SetNextItemWidth", &MteGUIContext::SetNextItemWidth);

        // ========== Item rectangle ==========
        gui_type.set("GetItemRectMinX", &MteGUIContext::GetItemRectMinX);
        gui_type.set("GetItemRectMinY", &MteGUIContext::GetItemRectMinY);
        gui_type.set("GetItemRectMaxX", &MteGUIContext::GetItemRectMaxX);
        gui_type.set("GetItemRectMaxY", &MteGUIContext::GetItemRectMaxY);

        // ========== Invisible Button ==========
        gui_type.set("InvisibleButton", &MteGUIContext::InvisibleButton);
        gui_type.set("IsItemActive", &MteGUIContext::IsItemActive);
        gui_type.set("IsAnyItemActive", &MteGUIContext::IsAnyItemActive);
        gui_type.set("IsItemHovered", &MteGUIContext::IsItemHovered);

        // ========== ID Stack ==========
        gui_type.set("PushID", sol::overload(
            static_cast<void (MteGUIContext::*)(int)>(&MteGUIContext::PushID),
            static_cast<void (MteGUIContext::*)(const std::string&)>(&MteGUIContext::PushID)
        ));
        gui_type.set("PopID", &MteGUIContext::PopID);
    }

} // namespace RenderUI