#include "MagicUIContext.h"

namespace RenderUI {

    static std::function<void()> MakeLuaCallback(sol::function& onClick) {
        return [onClick]() {
            if (onClick.valid()) onClick();
            };
    }

    void MteGUIContext::BindCore(sol::state& lua, const std::string& name) {
        sol::table gui_type = lua[name];
        // ========== Basic text & labels ==========
        gui_type.set("Label", &MteGUIContext::Label);
        gui_type.set("TextWrapped", &MteGUIContext::TextWrapped);

        // ========== Buttons / Clickables ==========
        gui_type.set("Button", [](MteGUIContext& self, const std::string& label, sol::function onClick, float width = 0.0f, float height = 0.0f) -> bool {
            return self.Button(label, MakeLuaCallback(onClick), width, height);
            });
        gui_type.set("RadioButton", &MteGUIContext::RadioButton);
        gui_type.set("Selectable", [](MteGUIContext& self, const std::string& label, bool selected, sol::object flags, sol::object width, sol::object height) -> bool {
            return self.Selectable(label, selected,
                flags.is<int>() ? flags.as<int>() : 0,
                width.is<float>() ? width.as<float>() : 0.0f,
                height.is<float>() ? height.as<float>() : 0.0f);
            });

        // ========== Value Editors ==========
        gui_type.set("Checkbox", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key) {
            bool value = tbl[key].get_or(false);
            self.Checkbox(label, &value);
            tbl[key] = value;
            });
        gui_type.set("IntSlider", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, int min, int max) {
            int value = tbl[key].get_or(0);
            self.IntSlider(label, &value, min, max);
            tbl[key] = value;
            });
        gui_type.set("FloatSlider", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, float min, float max) {
            float value = tbl[key].get_or(0.0f);
            self.FloatSlider(label, &value, min, max);
            tbl[key] = value;
            });
        gui_type.set("DragFloat", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            float speed, float min, float max, const std::string& fmt, int power) -> bool {
                float value = tbl[key].get_or(0.0f);
                bool changed = self.DragFloat(label, &value, speed, min, max, fmt.c_str(), power);
                if (changed) tbl[key] = value;
                return changed;
            });
        gui_type.set("DragInt", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            float speed, int min, int max, const std::string& fmt) -> bool {
                int value = tbl[key].get_or(0);
                bool changed = self.DragInt(label, &value, speed, min, max, fmt.c_str());
                if (changed) tbl[key] = value;
                return changed;
            });
        gui_type.set("TextInput", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, size_t bufferSize) {
            std::string text = tbl[key].get_or(std::string(""));
            text.resize(bufferSize > 0 ? bufferSize : 256);
            self.TextInput(label, text.data(), text.size());
            text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
            tbl[key] = text;
            });
        gui_type.set("TextArea", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key, size_t bufferSize) {
            std::string text = tbl[key].get_or(std::string(""));
            text.resize(bufferSize > 0 ? bufferSize : 1024);
            self.TextArea(label, text.data(), text.size());
            text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
            tbl[key] = text;
            });
        gui_type.set("InputTextWithHint", [](MteGUIContext& self, const std::string& label, const std::string& hint,
            sol::table tbl, const std::string& key, size_t bufferSize, int flags) -> bool {
                std::string text = tbl[key].get_or(std::string(""));
                text.resize(bufferSize > 0 ? bufferSize : 256);
                bool changed = self.InputTextWithHint(label, hint, text.data(), text.size(), flags);
                if (changed) {
                    text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
                    tbl[key] = text;
                }
                return changed;
            });
        gui_type.set("InputInt", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            int step, int stepFast, int flags) -> bool {
                int value = tbl[key].get_or(0);
                bool changed = self.InputInt(label, &value, step, stepFast, flags);
                if (changed) tbl[key] = value;
                return changed;
            });
        gui_type.set("InputFloat", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            float step, float stepFast, int flags) -> bool {
                float value = tbl[key].get_or(0.0f);
                bool changed = self.InputFloat(label, &value, step, stepFast, flags);
                if (changed) tbl[key] = value;
                return changed;
            });
        gui_type.set("InputTextMultiline", &MteGUIContext::InputTextMultiline);

        // ========== Color ==========
        gui_type.set("ColorEdit", [](MteGUIContext& self, const std::string& label, sol::table colorTable) {
            float color[4] = {
                colorTable[1].get_or(1.0f), colorTable[2].get_or(1.0f),
                colorTable[3].get_or(1.0f), colorTable[4].get_or(1.0f)
            };
            self.ColorEdit(label, color);
            colorTable[1] = color[0]; colorTable[2] = color[1];
            colorTable[3] = color[2]; colorTable[4] = color[3];
            });
        gui_type.set("ColorPicker", [](MteGUIContext& self, const std::string& label, sol::table colorTable, int flags) -> bool {
            float color[4] = {
                colorTable[1].get_or(1.0f), colorTable[2].get_or(1.0f),
                colorTable[3].get_or(1.0f), colorTable[4].get_or(1.0f)
            };
            bool changed = self.ColorPicker(label, color, flags);
            if (changed) {
                colorTable[1] = color[0]; colorTable[2] = color[1];
                colorTable[3] = color[2]; colorTable[4] = color[3];
            }
            return changed;
            });

        // ========== Vector controls ==========
        gui_type.set("Vector2Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
            float value[2] = { vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f) };
            self.Vector2Control(label, value, speed, labelWidth);
            vecTable[1] = value[0]; vecTable[2] = value[1];
            });
        gui_type.set("Vector3Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
            float value[3] = { vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f), vecTable[3].get_or(0.0f) };
            self.Vector3Control(label, value, speed, labelWidth);
            vecTable[1] = value[0]; vecTable[2] = value[1]; vecTable[3] = value[2];
            });
        gui_type.set("Vector4Control", [](MteGUIContext& self, const std::string& label, sol::table vecTable, float speed, float labelWidth) {
            float value[4] = { vecTable[1].get_or(0.0f), vecTable[2].get_or(0.0f), vecTable[3].get_or(0.0f), vecTable[4].get_or(0.0f) };
            self.Vector4Control(label, value, speed, labelWidth);
            vecTable[1] = value[0]; vecTable[2] = value[1]; vecTable[3] = value[2]; vecTable[4] = value[3];
            });

        // ========== Combo & Lists ==========
        gui_type.set("Combo", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            sol::table items, int popupMaxHeightInItems) -> bool {
                int currentItem = tbl[key].get_or(0);
                std::vector<std::string> itemList;
                for (size_t i = 1; i <= items.size(); ++i)
                    itemList.push_back(items[i].get_or(std::string("")));
                bool changed = self.Combo(label, &currentItem, itemList, popupMaxHeightInItems);
                if (changed) tbl[key] = currentItem;
                return changed;
            });
        gui_type.set("ListBox", [](MteGUIContext& self, const std::string& label, sol::table tbl, const std::string& key,
            sol::table items, int heightInItems) -> bool {
                int currentItem = tbl[key].get_or(0);
                std::vector<std::string> itemList;
                for (size_t i = 1; i <= items.size(); ++i)
                    itemList.push_back(items[i].get_or(std::string("")));
                bool changed = self.ListBox(label, &currentItem, itemList, heightInItems);
                if (changed) tbl[key] = currentItem;
                return changed;
            });

        // ========== Progress & Indicators ==========
        gui_type.set("ProgressBar", &MteGUIContext::ProgressBar);
    }

} // namespace RenderUI