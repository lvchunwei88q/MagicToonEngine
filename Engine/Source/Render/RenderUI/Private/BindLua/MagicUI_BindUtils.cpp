#include <MagicUIContext.h>

namespace RenderUI {

    void MteGUIContext::BindUtils(sol::state& lua, const std::string& name) {
        sol::table gui_type = lua[name];
        // ========== Misc Helpers ==========
        gui_type.set("CheckboxFlags", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, unsigned int flagValue) -> bool {
            unsigned int flags = tbl[key].get_or(0u);
            bool changed = self.CheckboxFlags(label, &flags, flagValue);
            if (changed) tbl[key] = flags;
            return changed;
            });

        // ========== Viewport ==========
        gui_type.set("GetMainViewportBounds", [](MteGUIContext& self) {
            float x, y, w, h;
            self.GetMainViewportBounds(&x, &y, &w, &h);
            return std::make_tuple(x, y, w, h);
            });

        // ========== Style API ==========
        gui_type.set("PushStyleColor", &MteGUIContext::PushStyleColor);
        gui_type.set("PopStyleColor", &MteGUIContext::PopStyleColor);
        gui_type.set("PushStyleVarFloat", &MteGUIContext::PushStyleVarFloat);
        gui_type.set("PushStyleVarVec2", &MteGUIContext::PushStyleVarVec2);
        gui_type.set("PopStyleVar", &MteGUIContext::PopStyleVar);
        gui_type.set("BeginDisabled", &MteGUIContext::BeginDisabled);
        gui_type.set("EndDisabled", &MteGUIContext::EndDisabled);

        // ========== Drag and Drop ==========
        gui_type.set("BeginDragDropSource", &MteGUIContext::BeginDragDropSource);
        gui_type.set("SetDragDropPayload", sol::overload(
            [](MteGUIContext& self, const std::string& type, uint64_t data) -> bool {
                return self.SetDragDropPayload(type, data);
            },
            [](MteGUIContext& self, const std::string& type, const std::string& data) -> bool {
                return self.SetDragDropPayload(type, data);
            }
        ));
        gui_type.set("EndDragDropSource", &MteGUIContext::EndDragDropSource);
        gui_type.set("BeginDragDropTarget", &MteGUIContext::BeginDragDropTarget);
        gui_type.set("AcceptDragDropPayload", sol::overload(
            [](MteGUIContext& self, const std::string& type, sol::table resultTbl) -> bool {
                uint64_t outData = 0;
                bool accepted = self.AcceptDragDropPayload(type, &outData);
                if (accepted) resultTbl["data"] = outData;
                return accepted;
            },
            [](MteGUIContext& self, const std::string& type, sol::table resultTbl, bool isString) -> bool {
                std::string outData;
                bool accepted = self.AcceptDragDropPayload(type, &outData);
                if (accepted) resultTbl["data"] = outData;
                return accepted;
            }
        ));
        gui_type.set("AcceptAnyDragDropPayload", [](MteGUIContext& self, sol::table resultTbl) -> bool {
            std::string outType;
            uint64_t outU64 = 0;
            std::string outStr;
            bool outIsU64 = false;
            bool accepted = self.AcceptAnyDragDropPayload(&outType, &outU64, &outStr, &outIsU64);
            if (accepted) {
                resultTbl["type"] = outType;
                resultTbl["isU64"] = outIsU64;
                if (outIsU64) resultTbl["data"] = outU64;
                else resultTbl["data"] = outStr;
            }
            return accepted;
            });
        gui_type.set("EndDragDropTarget", &MteGUIContext::EndDragDropTarget);

        // ========== Mouse Cursor ==========
        gui_type.set("SetMouseCursor", &MteGUIContext::SetMouseCursor);

        // ========== Input API ==========
        gui_type.set("IsMouseButtonDown", &MteGUIContext::IsMouseButtonDown);
        gui_type.set("IsMouseButtonClicked", &MteGUIContext::IsMouseButtonClicked);
        gui_type.set("IsMouseDoubleClicked", &MteGUIContext::IsMouseDoubleClicked);
        gui_type.set("IsMouseDragging", &MteGUIContext::IsMouseDragging);
        gui_type.set("GetMouseDragDeltaX", &MteGUIContext::GetMouseDragDeltaX);
        gui_type.set("GetMouseDragDeltaY", &MteGUIContext::GetMouseDragDeltaY);
        gui_type.set("ResetMouseDragDelta", &MteGUIContext::ResetMouseDragDelta);
        gui_type.set("GetMousePosX", &MteGUIContext::GetMousePosX);
        gui_type.set("GetMousePosY", &MteGUIContext::GetMousePosY);
        gui_type.set("GetMouseWheelDelta", &MteGUIContext::GetMouseWheelDelta);

        gui_type.set("IsKeyDown", &MteGUIContext::IsKeyDown);
        gui_type.set("IsKeyPressed", &MteGUIContext::IsKeyPressed);
        gui_type.set("IsKeyReleased", &MteGUIContext::IsKeyReleased);

        gui_type.set("WantTextInput", &MteGUIContext::WantTextInput);
        gui_type.set("CaptureMouseFromApp", &MteGUIContext::CaptureMouseFromApp);
        gui_type.set("CaptureKeyboardFromApp", &MteGUIContext::CaptureKeyboardFromApp);

        // ========== Focus & Activation ==========
        gui_type.set("SetKeyboardFocusHere", &MteGUIContext::SetKeyboardFocusHere);
        gui_type.set("IsItemDeactivated", &MteGUIContext::IsItemDeactivated);
        gui_type.set("IsItemDeactivatedAfterEdit", &MteGUIContext::IsItemDeactivatedAfterEdit);

        // ========== Clipboard ==========
        gui_type.set("SetClipboardText", &MteGUIContext::SetClipboardText);
        gui_type.set("GetClipboardText", &MteGUIContext::GetClipboardText);
    }

} // namespace RenderUI