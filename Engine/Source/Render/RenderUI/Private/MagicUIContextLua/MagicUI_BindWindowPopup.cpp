#include <MagicUIContext.h>

namespace RenderUI {

    void MteGUIContext::BindWindowPopup(sol::state& lua, const std::string& name) {
        sol::table gui_type = lua[name];
        // ========== Windows ==========
        gui_type.set("BeginWindow", [](MteGUIContext& self, const std::string& name, sol::object open, int flags) -> bool {
            if (open.is<sol::table>()) {
                sol::table tbl = open;
                bool openVal = tbl["open"].get_or(true);
                bool result = self.BeginWindow(name, &openVal, flags);
                tbl["open"] = openVal;
                return result;
            }
            return self.BeginWindow(name, nullptr, flags);
            });
        gui_type.set("EndWindow", &MteGUIContext::EndWindow);
        gui_type.set("SetNextWindowSize", &MteGUIContext::SetNextWindowSize);
        gui_type.set("SetNextWindowPos", &MteGUIContext::SetNextWindowPos);
        gui_type.set("SetNextWindowFocus", &MteGUIContext::SetNextWindowFocus);
        gui_type.set("SetWindowFocus", &MteGUIContext::SetWindowFocus);
        gui_type.set("IsWindowFocused", &MteGUIContext::IsWindowFocused);
        gui_type.set("IsWindowHovered", &MteGUIContext::IsWindowHovered);
        gui_type.set("SetWindowFontScale", &MteGUIContext::SetWindowFontScale);

        // ========== Pop-ups & Tooltips ==========
        gui_type.set("OpenPopup", &MteGUIContext::OpenPopup);
        gui_type.set("BeginPopup", &MteGUIContext::BeginPopup);
        gui_type.set("BeginPopupModal", &MteGUIContext::BeginPopupModal);
        gui_type.set("BeginPopupContextItem", &MteGUIContext::BeginPopupContextItem);
        gui_type.set("BeginPopupContextWindow", &MteGUIContext::BeginPopupContextWindow);
        gui_type.set("EndPopup", &MteGUIContext::EndPopup);
        gui_type.set("CloseCurrentPopup", &MteGUIContext::CloseCurrentPopup);
        gui_type.set("BeginTooltip", &MteGUIContext::BeginTooltip);
        gui_type.set("EndTooltip", &MteGUIContext::EndTooltip);
        gui_type.set("SetTooltip", &MteGUIContext::SetTooltip);

        // ========== Tree & Collapsing Sections ==========
        gui_type.set("TreeNode", &MteGUIContext::TreeNode);
        gui_type.set("TreeNodeEx", &MteGUIContext::TreeNodeEx);
        gui_type.set("TreePop", &MteGUIContext::TreePop);
        gui_type.set("SetNextItemOpen", &MteGUIContext::SetNextItemOpen);
        gui_type.set("SetNextItemAllowOverlap", &MteGUIContext::SetNextItemAllowOverlap);
        gui_type.set("CollapsingHeader", &MteGUIContext::CollapsingHeader);
        gui_type.set("IsItemClicked", &MteGUIContext::IsItemClicked);

        // ========== Tab Bars ==========
        gui_type.set("BeginTabBar", &MteGUIContext::BeginTabBar);
        gui_type.set("EndTabBar", &MteGUIContext::EndTabBar);
        gui_type.set("BeginTabItem", [](MteGUIContext& self, const std::string& label, sol::object open) -> bool {
            if (open.is<sol::table>()) {
                sol::table tbl = open;
                bool* pOpen = nullptr;
                bool openVal = false;
                if (tbl["open"].valid()) {
                    openVal = tbl["open"];
                    pOpen = &openVal;
                }
                bool result = self.BeginTabItem(label, pOpen);
                if (pOpen) tbl["open"] = openVal;
                return result;
            }
            return self.BeginTabItem(label);
            });
        gui_type.set("EndTabItem", &MteGUIContext::EndTabItem);

        // ========== Main-Menu / Menus ==========
        gui_type.set("BeginMainMenuBar", &MteGUIContext::BeginMainMenuBar);
        gui_type.set("EndMainMenuBar", &MteGUIContext::EndMainMenuBar);
        gui_type.set("BeginMenu", &MteGUIContext::BeginMenu);
        gui_type.set("EndMenu", &MteGUIContext::EndMenu);
        gui_type.set("MenuItem", &MteGUIContext::MenuItem);
    }

} // namespace RenderUI