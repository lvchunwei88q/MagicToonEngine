#include <MagicUIContext.h>

namespace RenderUI {
    void MteGUIContext::BindLuaFunction(sol::state& lua)
    {
        MteGUIContext& content = MteGUIContext::Get();

        // 注册类型
        auto gui_type = lua.new_usertype<MteGUIContext>(
            "GUI",
            sol::no_constructor,

            // ========== Basic text & labels ==========
            "Label", &MteGUIContext::Label,
            "TextWrapped", &MteGUIContext::TextWrapped,

            // ========== Buttons / Clickables ==========
            // 注意：Button的回调函数在Lua中直接传递Lua函数
            "Button", [](MteGUIContext& self, const std::string& label, sol::function onClick, float width = 0.0f, float height = 0.0f) -> bool {
                return self.Button(label, [onClick]() {
                    if (onClick.valid()) {
                        onClick();
                    }
                    }, width, height);
            },
            "RadioButton", &MteGUIContext::RadioButton,
            "Selectable", [](MteGUIContext& self, const std::string& label, bool selected, sol::object flags, sol::object width, sol::object height) -> bool {
                return self.Selectable(label, selected,
                    flags.is<int>() ? flags.as<int>() : 0,
                    width.is<float>() ? width.as<float>() : 0.0f,
                    height.is<float>() ? height.as<float>() : 0.0f);
            },

            // ========== Value Editors ==========
            // Checkbox - 从Lua表读取布尔值并更新
            "Checkbox", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key) {
                bool value = tbl[key].get_or(false);
                self.Checkbox(label, &value);
                tbl[key] = value;
            },

            // IntSlider - 从Lua表读取/更新
            "IntSlider", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, int min, int max) {
                int value = tbl[key].get_or(0);
                self.IntSlider(label, &value, min, max);
                tbl[key] = value;
            },

            // FloatSlider
            "FloatSlider", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, float min, float max) {
                float value = tbl[key].get_or(0.0f);
                self.FloatSlider(label, &value, min, max);
                tbl[key] = value;
            },

            // DragFloat
            "DragFloat", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                float speed, float min, float max, const std::string& fmt, float power) -> bool {
                    float value = tbl[key].get_or(0.0f);
                    bool changed = self.DragFloat(label, &value, speed, min, max, fmt.c_str(), power);
                    if (changed) tbl[key] = value;
                    return changed;
            },

            // DragInt
            "DragInt", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                float speed, int min, int max, const std::string& fmt) -> bool {
                    int value = tbl[key].get_or(0);
                    bool changed = self.DragInt(label, &value, speed, min, max, fmt.c_str());
                    if (changed) tbl[key] = value;
                    return changed;
            },

            // TextInput - 在Lua中更自然地处理字符串
            "TextInput", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, size_t bufferSize) {
                std::string text = tbl[key].get_or(std::string(""));
                text.resize(bufferSize > 0 ? bufferSize : 256);
                self.TextInput(label, text.data(), text.size());
                // 去除尾部的空字符
                text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
                tbl[key] = text;
            },

            // TextArea
            "TextArea", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, size_t bufferSize) {
                std::string text = tbl[key].get_or(std::string(""));
                text.resize(bufferSize > 0 ? bufferSize : 1024);
                self.TextArea(label, text.data(), text.size());
                text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
                tbl[key] = text;
            },

            // InputTextWithHint
            "InputTextWithHint", [](MteGUIContext& self, const std::string& label, const std::string& hint,
                sol::table tbl, const std::string& key, size_t bufferSize, int flags) -> bool {
                    std::string text = tbl[key].get_or(std::string(""));
                    text.resize(bufferSize > 0 ? bufferSize : 256);
                    bool changed = self.InputTextWithHint(label, hint, text.data(), text.size(), flags);
                    if (changed) {
                        text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
                        tbl[key] = text;
                    }
                    return changed;
            },

            // InputInt
            "InputInt", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                int step, int stepFast, int flags) -> bool {
                    int value = tbl[key].get_or(0);
                    bool changed = self.InputInt(label, &value, step, stepFast, flags);
                    if (changed) tbl[key] = value;
                    return changed;
            },

            // InputFloat
            "InputFloat", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                float step, float stepFast, int flags) -> bool {
                    float value = tbl[key].get_or(0.0f);
                    bool changed = self.InputFloat(label, &value, step, stepFast, flags);
                    if (changed) tbl[key] = value;
                    return changed;
            },

            // ========== Color ==========
            // ColorEdit - 使用Lua表 {r, g, b, a}
            "ColorEdit", [](MteGUIContext& self, const std::string& label, sol::table colorTable) {
                float color[4] = {
                    colorTable[1].get_or(1.0f),
                    colorTable[2].get_or(1.0f),
                    colorTable[3].get_or(1.0f),
                    colorTable[4].get_or(1.0f)
                };
                self.ColorEdit(label, color);
                // 更新回Lua表
                colorTable[1] = color[0];
                colorTable[2] = color[1];
                colorTable[3] = color[2];
                colorTable[4] = color[3];
            },

            // ColorPicker
            "ColorPicker", [](MteGUIContext& self, const std::string& label, sol::table colorTable, int flags) -> bool {
                float color[4] = {
                    colorTable[1].get_or(1.0f),
                    colorTable[2].get_or(1.0f),
                    colorTable[3].get_or(1.0f),
                    colorTable[4].get_or(1.0f)
                };
                bool changed = self.ColorPicker(label, color, flags);
                if (changed) {
                    colorTable[1] = color[0];
                    colorTable[2] = color[1];
                    colorTable[3] = color[2];
                    colorTable[4] = color[3];
                }
                return changed;
            },

            // Vector controls
            "Vector2Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
                float value[2] = { vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f) };
                self.Vector2Control(label, value, speed, labelWidth);
                vecTable[1] = value[0];
                vecTable[2] = value[1];
            },

            "Vector3Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
                float value[3] = { vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f), vecTable[3].get_or(0.0f) };
                self.Vector3Control(label, value, speed, labelWidth);
                vecTable[1] = value[0];
                vecTable[2] = value[1];
                vecTable[3] = value[2];
            },

            "Vector4Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
                float value[4] = {
                    vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f),
                    vecTable[3].get_or(0.0f), vecTable[4].get_or(0.0f)
                };
                self.Vector4Control(label, value, speed, labelWidth);
                vecTable[1] = value[0];
                vecTable[2] = value[1];
                vecTable[3] = value[2];
                vecTable[4] = value[3];
            },

            // ========== Combo & Lists ==========
            "Combo", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                sol::table items, int popupMaxHeightInItems) -> bool {
                    int currentItem = tbl[key].get_or(0);
                    std::vector<std::string> itemList;
                    for (size_t i = 1; i <= items.size(); ++i) {
                        itemList.push_back(items[i].get_or(std::string("")));
                    }
                    bool changed = self.Combo(label, &currentItem, itemList, popupMaxHeightInItems);
                    if (changed) tbl[key] = currentItem;
                    return changed;
            },

            "ListBox", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
                sol::table items, int heightInItems) -> bool {
                    int currentItem = tbl[key].get_or(0);
                    std::vector<std::string> itemList;
                    for (size_t i = 1; i <= items.size(); ++i) {
                        itemList.push_back(items[i].get_or(std::string("")));
                    }
                    bool changed = self.ListBox(label, &currentItem, itemList, heightInItems);
                    if (changed) tbl[key] = currentItem;
                    return changed;
            },

            // ========== Progress & Indicators ==========
            "ProgressBar", &MteGUIContext::ProgressBar,

            // ========== Layout Helpers ==========
            "BeginGroup", &MteGUIContext::BeginGroup,
            "EndGroup", &MteGUIContext::EndGroup,
            "SameLine", &MteGUIContext::SameLine,
            "AlignTextToFramePadding", &MteGUIContext::AlignTextToFramePadding,
            "SetScrollHereY", &MteGUIContext::SetScrollHereY,
            "GetScrollY", &MteGUIContext::GetScrollY,
            "GetScrollMaxY", &MteGUIContext::GetScrollMaxY,
            "Separator", &MteGUIContext::Separator,
            "Spacing", &MteGUIContext::Spacing,
            "Dummy", &MteGUIContext::Dummy,
            "NewLine", &MteGUIContext::NewLine,

            // ========== Tree & Collapsing Sections ==========
            "TreeNode", &MteGUIContext::TreeNode,
            "TreeNodeEx", &MteGUIContext::TreeNodeEx,
            "TreePop", &MteGUIContext::TreePop,
            "SetNextItemOpen", &MteGUIContext::SetNextItemOpen,
            "SetNextItemAllowOverlap", &MteGUIContext::SetNextItemAllowOverlap,
            "CollapsingHeader", &MteGUIContext::CollapsingHeader,
            "IsItemClicked", &MteGUIContext::IsItemClicked,

            // ========== Tab Bars ==========
            "BeginTabBar", &MteGUIContext::BeginTabBar,
            "EndTabBar", &MteGUIContext::EndTabBar,
            "BeginTabItem", [](MteGUIContext& self, const std::string& label, sol::object open) -> bool {
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
            },
            "EndTabItem", &MteGUIContext::EndTabItem,

            // ========== Main-Menu / Menus ==========
            "BeginMainMenuBar", &MteGUIContext::BeginMainMenuBar,
            "EndMainMenuBar", &MteGUIContext::EndMainMenuBar,
            "BeginMenu", &MteGUIContext::BeginMenu,
            "EndMenu", &MteGUIContext::EndMenu,
            "MenuItem", &MteGUIContext::MenuItem,

            // ========== Child Regions & Windows ==========
            "BeginChild", &MteGUIContext::BeginChild,
            "EndChild", &MteGUIContext::EndChild,

            // ========== Pop-ups & Tooltips ==========
            "OpenPopup", &MteGUIContext::OpenPopup,
            "BeginPopup", &MteGUIContext::BeginPopup,
            "BeginPopupModal", &MteGUIContext::BeginPopupModal,
            "BeginPopupContextItem", &MteGUIContext::BeginPopupContextItem,
            "BeginPopupContextWindow", &MteGUIContext::BeginPopupContextWindow,
            "EndPopup", &MteGUIContext::EndPopup,
            "CloseCurrentPopup", &MteGUIContext::CloseCurrentPopup,
            "BeginTooltip", &MteGUIContext::BeginTooltip,
            "EndTooltip", &MteGUIContext::EndTooltip,
            "SetTooltip", &MteGUIContext::SetTooltip,

            // ========== Images ==========
            "Image", &MteGUIContext::Image,
            "ImageButton", &MteGUIContext::ImageButton,

            // ========== Tables ==========
            "BeginTable", &MteGUIContext::BeginTable,
            "EndTable", &MteGUIContext::EndTable,
            "TableSetupColumn", &MteGUIContext::TableSetupColumn,
            "TableHeadersRow", &MteGUIContext::TableHeadersRow,
            "TableNextRow", &MteGUIContext::TableNextRow,
            "TableSetColumnIndex", &MteGUIContext::TableSetColumnIndex,
            "TableNextColumn", &MteGUIContext::TableNextColumn,

            // ========== Misc Helpers ==========
            "CheckboxFlags", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, unsigned int flagValue) -> bool {
                unsigned int flags = tbl[key].get_or(0u);
                bool changed = self.CheckboxFlags(label, &flags, flagValue);
                if (changed) tbl[key] = flags;
                return changed;
            },

            "SetNextItemWidth", &MteGUIContext::SetNextItemWidth,
            "SetNextWindowSize", &MteGUIContext::SetNextWindowSize,
            "SetNextWindowPos", &MteGUIContext::SetNextWindowPos,
            "SetNextWindowFocus", &MteGUIContext::SetNextWindowFocus,
            "SetWindowFocus", &MteGUIContext::SetWindowFocus,

            // GetMainViewportBounds - 返回多个值
            "GetMainViewportBounds", [](MteGUIContext& self) {
                float x, y, w, h;
                self.GetMainViewportBounds(&x, &y, &w, &h);
                return std::make_tuple(x, y, w, h);
            },

            "BeginWindow", [](MteGUIContext& self, const std::string& name, sol::object open, int flags) -> bool {
                if (open.is<sol::table>()) {
                    sol::table tbl = open;
                    bool openVal = tbl["open"].get_or(true);
                    bool result = self.BeginWindow(name, &openVal, flags);
                    tbl["open"] = openVal;
                    return result;
                }
                return self.BeginWindow(name, nullptr, flags);
            },
            "EndWindow", &MteGUIContext::EndWindow,

            // ========== Layout Query ==========
            "CalcTextWidth", &MteGUIContext::CalcTextWidth,
            "GetContentRegionAvailWidth", &MteGUIContext::GetContentRegionAvailWidth,
            "GetContentRegionAvailHeight", &MteGUIContext::GetContentRegionAvailHeight,
            "GetCursorPosX", &MteGUIContext::GetCursorPosX,
            "GetCursorPosY", &MteGUIContext::GetCursorPosY,
            "SetCursorPosX", &MteGUIContext::SetCursorPosX,
            "SetCursorPosY", &MteGUIContext::SetCursorPosY,
            "GetWindowPosX", &MteGUIContext::GetWindowPosX,
            "GetWindowPosY", &MteGUIContext::GetWindowPosY,
            "GetWindowWidth", &MteGUIContext::GetWindowWidth,

            // Item rectangle
            "GetItemRectMinX", &MteGUIContext::GetItemRectMinX,
            "GetItemRectMinY", &MteGUIContext::GetItemRectMinY,
            "GetItemRectMaxX", &MteGUIContext::GetItemRectMaxX,
            "GetItemRectMaxY", &MteGUIContext::GetItemRectMaxY,

            // ========== Invisible Button ==========
            "InvisibleButton", &MteGUIContext::InvisibleButton,
            "IsItemActive", &MteGUIContext::IsItemActive,
            "IsAnyItemActive", &MteGUIContext::IsAnyItemActive,
            "IsItemHovered", &MteGUIContext::IsItemHovered,

            // ========== Focus & Activation ==========
            "SetKeyboardFocusHere", &MteGUIContext::SetKeyboardFocusHere,
            "IsItemDeactivated", &MteGUIContext::IsItemDeactivated,
            "IsItemDeactivatedAfterEdit", &MteGUIContext::IsItemDeactivatedAfterEdit,

            "GetMouseDragDeltaY", &MteGUIContext::GetMouseDragDeltaY,
            "ResetMouseDragDelta", &MteGUIContext::ResetMouseDragDelta,

            // ========== ID Stack ==========
            "PushID", sol::overload(
                static_cast<void (MteGUIContext::*)(int)>(&MteGUIContext::PushID),
                static_cast<void (MteGUIContext::*)(const std::string&)>(&MteGUIContext::PushID)
            ),
            "PopID", &MteGUIContext::PopID,

            // ========== Style API ==========
            "PushStyleColor", &MteGUIContext::PushStyleColor,
            "PopStyleColor", &MteGUIContext::PopStyleColor,
            "PushStyleVarFloat", &MteGUIContext::PushStyleVarFloat,
            "PushStyleVarVec2", &MteGUIContext::PushStyleVarVec2,
            "PopStyleVar", &MteGUIContext::PopStyleVar,
            "BeginDisabled", &MteGUIContext::BeginDisabled,
            "EndDisabled", &MteGUIContext::EndDisabled,

            // ========== Drag and Drop ==========
            "BeginDragDropSource", &MteGUIContext::BeginDragDropSource,
            "SetDragDropPayload", sol::overload(
                [](MteGUIContext& self, const std::string& type, uint64_t data) -> bool {
                    return self.SetDragDropPayload(type, data);
                },
                [](MteGUIContext& self, const std::string& type, const std::string& data) -> bool {
                    return self.SetDragDropPayload(type, data);
                }
            ),
            "EndDragDropSource", &MteGUIContext::EndDragDropSource,
            "BeginDragDropTarget", &MteGUIContext::BeginDragDropTarget,
            "AcceptDragDropPayload", sol::overload(
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
            ),
            "AcceptAnyDragDropPayload", [](MteGUIContext& self, sol::table resultTbl) -> bool {
                std::string outType;
                uint64_t outU64 = 0;
                std::string outStr;
                bool outIsU64 = false;
                bool accepted = self.AcceptAnyDragDropPayload(&outType, &outU64, &outStr, &outIsU64);
                if (accepted) {
                    resultTbl["type"] = outType;
                    resultTbl["isU64"] = outIsU64;
                    if (outIsU64) {
                        resultTbl["data"] = outU64;
                    }
                    else {
                        resultTbl["data"] = outStr;
                    }
                }
                return accepted;
            },
            "EndDragDropTarget", &MteGUIContext::EndDragDropTarget,

            // ========== Mouse Cursor ==========
            "SetMouseCursor", &MteGUIContext::SetMouseCursor,

            // ========== Scene View Input API ==========
            "IsMouseButtonDown", &MteGUIContext::IsMouseButtonDown,
            "IsMouseButtonClicked", &MteGUIContext::IsMouseButtonClicked,
            "IsMouseDoubleClicked", &MteGUIContext::IsMouseDoubleClicked,
            "IsMouseDragging", &MteGUIContext::IsMouseDragging,
            "GetMouseDragDeltaX", &MteGUIContext::GetMouseDragDeltaX,
            "GetMousePosX", &MteGUIContext::GetMousePosX,
            "GetMousePosY", &MteGUIContext::GetMousePosY,
            "GetMouseWheelDelta", &MteGUIContext::GetMouseWheelDelta,

            "IsKeyDown", &MteGUIContext::IsKeyDown,
            "IsKeyPressed", &MteGUIContext::IsKeyPressed,
            "IsKeyReleased", &MteGUIContext::IsKeyReleased,

            "IsWindowFocused", &MteGUIContext::IsWindowFocused,
            "IsWindowHovered", &MteGUIContext::IsWindowHovered,
            "WantTextInput", &MteGUIContext::WantTextInput,

            "CaptureMouseFromApp", &MteGUIContext::CaptureMouseFromApp,
            "CaptureKeyboardFromApp", &MteGUIContext::CaptureKeyboardFromApp,

            "SetClipboardText", &MteGUIContext::SetClipboardText,
            "GetClipboardText", &MteGUIContext::GetClipboardText,

            "InputTextMultiline", &MteGUIContext::InputTextMultiline,
            "SetWindowFontScale", &MteGUIContext::SetWindowFontScale,

            // ========== Draw List Primitives ==========
            "DrawRect", &MteGUIContext::DrawRect,
            "DrawFilledRect", &MteGUIContext::DrawFilledRect,
            "DrawFilledRectRotated", &MteGUIContext::DrawFilledRectRotated,
            "DrawLine", &MteGUIContext::DrawLine,
            "DrawCircle", &MteGUIContext::DrawCircle,
            "DrawFilledCircle", &MteGUIContext::DrawFilledCircle,
            "DrawImageRect", &MteGUIContext::DrawImageRect,
            "DrawText", &MteGUIContext::DrawText,
            "DrawTextAligned", &MteGUIContext::DrawTextAligned,
            "DrawTextRotated90Aligned", &MteGUIContext::DrawTextRotated90Aligned,
            "DrawTextExAligned", &MteGUIContext::DrawTextExAligned,

            "CalcTextSizeA", [](MteGUIContext& self, const std::string& text, float fontSize,
                const std::string& fontPath, float lineHeight, float letterSpacing) -> std::pair<float, float> {
                    return self.CalcTextSizeA(text, fontSize, fontPath, lineHeight, letterSpacing);
            },

            "CalcTextSizeWrappedA", [](MteGUIContext& self, const std::string& text, float fontSize, float wrapWidth,
                const std::string& fontPath, float lineHeight, float letterSpacing) -> std::pair<float, float> {
                    return self.CalcTextSizeWrappedA(text, fontSize, wrapWidth, fontPath, lineHeight, letterSpacing);
            },

            "PushDrawListClipRect", &MteGUIContext::PushDrawListClipRect,
            "PopDrawListClipRect", &MteGUIContext::PopDrawListClipRect
        );

        lua["GUI"] = &content;
    }
}