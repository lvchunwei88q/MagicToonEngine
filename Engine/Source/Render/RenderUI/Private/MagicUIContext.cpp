#include <windows.h>

#include <MagicUIContext.h>
#include <Tools/TextLayout.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <imgui_internal.h>
#include <type_traits>

namespace RenderUI
{
    namespace
    {
        ImTextureID ToImTextureID(uint64_t textureId)
        {
            if constexpr (std::is_pointer_v<ImTextureID>) {
                return (ImTextureID)(static_cast<uintptr_t>(textureId));
            }
            return static_cast<ImTextureID>(textureId);
        }

        float ResolveFontSize(float fontSize)
        {
            return textlayout::ResolveFontSize(fontSize);
        }
    } // namespace

    /* basic text & labels */
    void MteGUIContext::Label(const std::string& text)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(text.c_str());
    }

    void MteGUIContext::TextWrapped(const std::string& text)
    {
        ImGui::TextWrapped("%s", text.c_str());
    }

    /* buttons / clickables */
    bool MteGUIContext::Button(const std::string& label, std::function<void()> onClick, float width, float height)
    {
        bool clicked = ImGui::Button(label.c_str(), ImVec2(width, height));
        if (clicked && onClick)
            onClick();
        return clicked;
    }

    bool MteGUIContext::RadioButton(const std::string& label, bool active)
    {
        return ImGui::RadioButton(label.c_str(), active);
    }

    bool MteGUIContext::Selectable(const std::string& label, bool selected, int flags, float width, float height)
    {
        return ImGui::Selectable(label.c_str(), selected, flags, ImVec2(width, height));
    }

    /* value editors */
    void MteGUIContext::Checkbox(const std::string& label, bool* value)
    {
        ImGui::Checkbox(label.c_str(), value);
    }

    void MteGUIContext::IntSlider(const std::string& label, int* value, int min, int max)
    {
        ImGui::SliderInt(label.c_str(), value, min, max);
    }

    void MteGUIContext::FloatSlider(const std::string& label, float* value, float min, float max)
    {
        ImGui::SliderFloat(label.c_str(), value, min, max);
    }

    bool MteGUIContext::DragFloat(const std::string& label, float* value, float speed, float min, float max,
        const char* fmt, float power)
    {
        CompensateWarp();
        bool changed = ImGui::DragFloat(label.c_str(), value, speed, min, max, fmt, power);
        HandleDragCapture();
        return changed;
    }

    bool MteGUIContext::DragInt(const std::string& label, int* value, float speed, int min, int max, const char* fmt)
    {
        CompensateWarp();
        bool changed = ImGui::DragInt(label.c_str(), value, speed, min, max, fmt);
        HandleDragCapture();
        return changed;
    }

    void MteGUIContext::TextInput(const std::string& label, char* buffer, size_t bufferSize)
    {
        ImGui::InputText(label.c_str(), buffer, bufferSize);
    }

    void MteGUIContext::TextArea(const std::string& label, char* buffer, size_t bufferSize)
    {
        ImGui::InputTextMultiline(label.c_str(), buffer, bufferSize, ImVec2(-FLT_MIN, 100));
    }

    bool MteGUIContext::InputTextWithHint(const std::string& label, const std::string& hint, char* buffer,
        size_t bufferSize, int flags)
    {
        return ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, bufferSize, flags);
    }

    bool MteGUIContext::InputInt(const std::string& label, int* value, int step, int stepFast, int flags)
    {
        return ImGui::InputInt(label.c_str(), value, step, stepFast, flags);
    }

    bool MteGUIContext::InputFloat(const std::string& label, float* value, float step, float stepFast, int flags)
    {
        return ImGui::InputFloat(label.c_str(), value, step, stepFast, "%.3f", static_cast<ImGuiInputTextFlags>(flags));
    }
    void MteGUIContext::ColorEdit(const std::string& label, float color[4])
    {
        ImGui::ColorEdit4(label.c_str(), color);
    }

    bool MteGUIContext::ColorPicker(const std::string& label, float color[4], int flags)
    {
        return ImGui::ColorPicker4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));
    }

    // Unity-style helper: label on the left, DragFloatN on the right
    static void LabeledDragFloatN(const char* label, float* value, int components, float speed, float labelWidth = 0.0f)
    {
        if (labelWidth <= 0.0f)
            labelWidth = ImGui::CalcTextSize(label).x + 20.0f;
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine(labelWidth);
        float avail = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        std::string hiddenLabel = std::string("##") + label;
        switch (components) {
        case 2:
            ImGui::DragFloat2(hiddenLabel.c_str(), value, speed);
            break;
        case 3:
            ImGui::DragFloat3(hiddenLabel.c_str(), value, speed);
            break;
        case 4:
            ImGui::DragFloat4(hiddenLabel.c_str(), value, speed);
            break;
        default:
            ImGui::DragFloat(hiddenLabel.c_str(), value, speed);
            break;
        }
    }

    void MteGUIContext::Vector2Control(const std::string& label, float value[2], float speed, float labelWidth)
    {
        CompensateWarp();
        LabeledDragFloatN(label.c_str(), value, 2, speed, labelWidth);
        HandleDragCapture();
    }

    void MteGUIContext::Vector3Control(const std::string& label, float value[3], float speed, float labelWidth)
    {
        CompensateWarp();
        LabeledDragFloatN(label.c_str(), value, 3, speed, labelWidth);
        HandleDragCapture();
    }

    void MteGUIContext::Vector4Control(const std::string& label, float value[4], float speed, float labelWidth)
    {
        CompensateWarp();
        LabeledDragFloatN(label.c_str(), value, 4, speed, labelWidth);
        HandleDragCapture();
    }

    /* combo & lists */
    bool MteGUIContext::Combo(const std::string& label, int* currentItem, const std::vector<std::string>& items,
        int popupMaxHeightInItems)
    {
        std::vector<const char*> cstrs;
        cstrs.reserve(items.size());
        for (const auto& s : items)
            cstrs.push_back(s.c_str());
        return ImGui::Combo(label.c_str(), currentItem, cstrs.data(), static_cast<int>(cstrs.size()),
            popupMaxHeightInItems);
    }

    bool MteGUIContext::ListBox(const std::string& label, int* currentItem, const std::vector<std::string>& items,
        int heightInItems)
    {
        std::vector<const char*> cstrs;
        cstrs.reserve(items.size());
        for (const auto& s : items)
            cstrs.push_back(s.c_str());
        return ImGui::ListBox(label.c_str(), currentItem, cstrs.data(), static_cast<int>(cstrs.size()), heightInItems);
    }

    /* progress & indicators */
    void MteGUIContext::ProgressBar(float fraction, float width, float height, const std::string& overlay)
    {
        ImGui::ProgressBar(fraction, ImVec2(width, height), overlay.c_str());
    }

    /* layout helpers */
    void MteGUIContext::BeginGroup(const std::string& name)
    {
        ImGui::BeginGroup();
        if (!name.empty())
            ImGui::TextUnformatted(name.c_str());
    }

    void MteGUIContext::EndGroup()
    {
        ImGui::EndGroup();
    }

    void MteGUIContext::SameLine(float offsetFromStartX, float spacing)
    {
        ImGui::SameLine(offsetFromStartX, spacing);
    }

    void MteGUIContext::AlignTextToFramePadding()
    {
        ImGui::AlignTextToFramePadding();
    }

    void MteGUIContext::SetScrollHereY(float centerYRatio)
    {
        ImGui::SetScrollHereY(centerYRatio);
    }

    float MteGUIContext::GetScrollY()
    {
        return ImGui::GetScrollY();
    }

    float MteGUIContext::GetScrollMaxY()
    {
        return ImGui::GetScrollMaxY();
    }

    void MteGUIContext::CloseCurrentPopup()
    {
        ImGui::CloseCurrentPopup();
    }

    void MteGUIContext::Separator()
    {
        ImGui::Separator();
    }

    void MteGUIContext::Spacing()
    {
        ImGui::Spacing();
    }

    void MteGUIContext::Dummy(float width, float height)
    {
        ImGui::Dummy(ImVec2(width, height));
    }

    void MteGUIContext::NewLine()
    {
        ImGui::NewLine();
    }

    /* tree & collapsing */
    bool MteGUIContext::TreeNode(const std::string& label)
    {
        return ImGui::TreeNode(label.c_str());
    }

    bool MteGUIContext::TreeNodeEx(const std::string& label, int flags)
    {
        return ImGui::TreeNodeEx(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags));
    }

    void MteGUIContext::TreePop()
    {
        ImGui::TreePop();
    }

    void MteGUIContext::SetNextItemOpen(bool is_open, int cond)
    {
        ImGui::SetNextItemOpen(is_open, cond);
    }

    void MteGUIContext::SetNextItemAllowOverlap()
    {
        ImGui::SetNextItemAllowOverlap();
    }

    bool MteGUIContext::CollapsingHeader(const std::string& label)
    {
        return ImGui::CollapsingHeader(label.c_str());
    }

    bool MteGUIContext::IsItemClicked(int mouseButton)
    {
        return ImGui::IsItemClicked(static_cast<ImGuiMouseButton>(mouseButton));
    }

    /* tab bars */
    bool MteGUIContext::BeginTabBar(const std::string& id)
    {
        return ImGui::BeginTabBar(id.c_str());
    }

    void MteGUIContext::EndTabBar()
    {
        ImGui::EndTabBar();
    }

    bool MteGUIContext::BeginTabItem(const std::string& label, bool* open)
    {
        return ImGui::BeginTabItem(label.c_str(), open);
    }

    void MteGUIContext::EndTabItem()
    {
        ImGui::EndTabItem();
    }

    /* main menu / menus */
    bool MteGUIContext::BeginMainMenuBar()
    {
        return ImGui::BeginMainMenuBar();
    }

    void MteGUIContext::EndMainMenuBar()
    {
        ImGui::EndMainMenuBar();
    }

    bool MteGUIContext::BeginMenu(const std::string& label, bool enabled)
    {
        return ImGui::BeginMenu(label.c_str(), enabled);
    }

    void MteGUIContext::EndMenu()
    {
        ImGui::EndMenu();
    }

    bool MteGUIContext::MenuItem(const std::string& label, const std::string& shortcut, bool selected, bool enabled)
    {
        return ImGui::MenuItem(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), selected, enabled);
    }

    /* child & windows */
    bool MteGUIContext::BeginChild(const std::string& id, float width, float height, bool border)
    {
        return ImGui::BeginChild(id.c_str(), ImVec2(width, height), border);
    }

    void MteGUIContext::EndChild()
    {
        ImGui::EndChild();
    }

    /* popups & tooltips */
    void MteGUIContext::OpenPopup(const std::string& id)
    {
        ImGui::OpenPopup(id.c_str());
    }

    bool MteGUIContext::BeginPopup(const std::string& id)
    {
        return ImGui::BeginPopup(id.c_str());
    }

    bool MteGUIContext::BeginPopupModal(const std::string& title, int flags)
    {
        return ImGui::BeginPopupModal(title.c_str(), nullptr, static_cast<ImGuiWindowFlags>(flags));
    }

    bool MteGUIContext::BeginPopupContextItem(const std::string& id, int mouseButton)
    {
        return ImGui::BeginPopupContextItem(id.empty() ? nullptr : id.c_str(), static_cast<ImGuiPopupFlags>(mouseButton));
    }

    bool MteGUIContext::BeginPopupContextWindow(const std::string& id, int mouseButton)
    {
        return ImGui::BeginPopupContextWindow(id.empty() ? nullptr : id.c_str(), static_cast<ImGuiPopupFlags>(mouseButton));
    }

    void MteGUIContext::EndPopup()
    {
        ImGui::EndPopup();
    }

    void MteGUIContext::BeginTooltip()
    {
        ImGui::BeginTooltip();
    }

    void MteGUIContext::EndTooltip()
    {
        ImGui::EndTooltip();
    }

    void MteGUIContext::SetTooltip(const std::string& text)
    {
        ImGui::SetTooltip("%s", text.c_str());
    }

    /* images */
    void MteGUIContext::Image(void* textureId, float width, float height, float uv0_x, float uv0_y, float uv1_x,
        float uv1_y)
    {
        if (!textureId)
            return;
        ImGui::Image(reinterpret_cast<ImTextureID>(textureId), ImVec2(width, height), ImVec2(uv0_x, uv0_y),
            ImVec2(uv1_x, uv1_y));
    }

    bool MteGUIContext::ImageButton(const std::string& id, void* textureId, float width, float height, float uv0_x,
        float uv0_y, float uv1_x, float uv1_y)
    {
        if (!textureId)
            return false;
        return ImGui::ImageButton(id.c_str(), reinterpret_cast<ImTextureID>(textureId), ImVec2(width, height),
            ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y));
    }

    /* tables */
    bool MteGUIContext::BeginTable(const std::string& id, int columns, int flags, float innerWidth)
    {
        return ImGui::BeginTable(id.c_str(), columns, flags, ImVec2(innerWidth, 0));
    }

    void MteGUIContext::EndTable()
    {
        ImGui::EndTable();
    }

    void MteGUIContext::TableSetupColumn(const std::string& label, int flags, float initWidthOrWeight, int userID)
    {
        ImGui::TableSetupColumn(label.c_str(), flags, initWidthOrWeight, userID);
    }

    void MteGUIContext::TableHeadersRow()
    {
        ImGui::TableHeadersRow();
    }

    void MteGUIContext::TableNextRow()
    {
        ImGui::TableNextRow();
    }

    void MteGUIContext::TableSetColumnIndex(int columnIndex)
    {
        ImGui::TableSetColumnIndex(columnIndex);
    }

    bool MteGUIContext::TableNextColumn()
    {
        return ImGui::TableNextColumn();
    }

    /* misc helpers */
    bool MteGUIContext::CheckboxFlags(const std::string& label, unsigned int* flags, unsigned int flagValue)
    {
        return ImGui::CheckboxFlags(label.c_str(), flags, flagValue);
    }

    void MteGUIContext::SetNextItemWidth(float width)
    {
        ImGui::SetNextItemWidth(width);
    }

    void MteGUIContext::SetNextWindowSize(float width, float height, int cond)
    {
        ImGui::SetNextWindowSize(ImVec2(width, height), static_cast<ImGuiCond>(cond));
    }

    void MteGUIContext::SetNextWindowPos(float x, float y, int cond, float pivot_x, float pivot_y)
    {
        ImGui::SetNextWindowPos(ImVec2(x, y), static_cast<ImGuiCond>(cond), ImVec2(pivot_x, pivot_y));
    }

    void MteGUIContext::SetNextWindowFocus()
    {
        ImGui::SetNextWindowFocus();
    }

    void MteGUIContext::SetWindowFocus()
    {
        ImGui::SetWindowFocus();
    }

    bool MteGUIContext::BeginWindow(const std::string& name, bool* open, int flags)
    {
        return ImGui::Begin(name.c_str(), open, flags);
    }

    void MteGUIContext::EndWindow()
    {
        ImGui::End();
    }

    /* layout query */
    float MteGUIContext::GetContentRegionAvailWidth()
    {
        return ImGui::GetContentRegionAvail().x;
    }

    float MteGUIContext::GetContentRegionAvailHeight()
    {
        return ImGui::GetContentRegionAvail().y;
    }

    float MteGUIContext::GetCursorPosX()
    {
        return ImGui::GetCursorPosX();
    }

    float MteGUIContext::GetCursorPosY()
    {
        return ImGui::GetCursorPosY();
    }

    void MteGUIContext::SetCursorPosX(float x)
    {
        ImGui::SetCursorPosX(x);
    }

    void MteGUIContext::SetCursorPosY(float y)
    {
        ImGui::SetCursorPosY(y);
    }

    float MteGUIContext::GetWindowPosX()
    {
        return ImGui::GetWindowPos().x;
    }

    float MteGUIContext::GetWindowPosY()
    {
        return ImGui::GetWindowPos().y;
    }

    float MteGUIContext::CalcTextWidth(const std::string& text)
    {
        return ImGui::CalcTextSize(text.c_str()).x;
    }

    float MteGUIContext::GetWindowWidth()
    {
        return ImGui::GetWindowWidth();
    }

    float MteGUIContext::GetItemRectMinX()
    {
        return ImGui::GetItemRectMin().x;
    }

    float MteGUIContext::GetItemRectMinY()
    {
        return ImGui::GetItemRectMin().y;
    }

    float MteGUIContext::GetItemRectMaxX()
    {
        return ImGui::GetItemRectMax().x;
    }

    float MteGUIContext::GetItemRectMaxY()
    {
        return ImGui::GetItemRectMax().y;
    }

    /* invisible button (for splitter) */
    bool MteGUIContext::InvisibleButton(const std::string& id, float width, float height)
    {
        return ImGui::InvisibleButton(id.c_str(), ImVec2(width, height));
    }

    bool MteGUIContext::IsItemActive()
    {
        return ImGui::IsItemActive();
    }

    bool MteGUIContext::IsAnyItemActive()
    {
        return ImGui::IsAnyItemActive();
    }

    bool MteGUIContext::IsItemHovered()
    {
        return ImGui::IsItemHovered();
    }

    /* focus & activation */
    void MteGUIContext::SetKeyboardFocusHere(int offset)
    {
        ImGui::SetKeyboardFocusHere(offset);
    }

    bool MteGUIContext::IsItemDeactivated()
    {
        return ImGui::IsItemDeactivated();
    }

    bool MteGUIContext::IsItemDeactivatedAfterEdit()
    {
        return ImGui::IsItemDeactivatedAfterEdit();
    }

    float MteGUIContext::GetMouseDragDeltaY(int button)
    {
        // Use 0.0f threshold for immediate response (no lock threshold)
        return ImGui::GetMouseDragDelta(button, 0.0f).y;
    }

    void MteGUIContext::ResetMouseDragDelta(int button)
    {
        ImGui::ResetMouseDragDelta(button);
    }

    /* ID stack */
    void MteGUIContext::PushID(int id)
    {
        ImGui::PushID(id);
    }

    void MteGUIContext::PushID(const std::string& id)
    {
        ImGui::PushID(id.c_str());
    }

    void MteGUIContext::PopID()
    {
        ImGui::PopID();
    }

    void MteGUIContext::PushStyleColor(int idx, float r, float g, float b, float a)
    {
        ImGui::PushStyleColor(static_cast<ImGuiCol_>(idx), ImVec4(r, g, b, a));
    }

    void MteGUIContext::PopStyleColor(int count)
    {
        ImGui::PopStyleColor(count);
    }

    void MteGUIContext::PushStyleVarFloat(int idx, float val)
    {
        ImGui::PushStyleVar(static_cast<ImGuiStyleVar_>(idx), val);
    }

    void MteGUIContext::PushStyleVarVec2(int idx, float x, float y)
    {
        ImGui::PushStyleVar(static_cast<ImGuiStyleVar_>(idx), ImVec2(x, y));
    }

    void MteGUIContext::PopStyleVar(int count)
    {
        ImGui::PopStyleVar(count);
    }

    void MteGUIContext::BeginDisabled(bool disabled)
    {
        ImGui::BeginDisabled(disabled);
    }

    void MteGUIContext::EndDisabled()
    {
        ImGui::EndDisabled();
    }

    /* Drag and Drop */
    bool MteGUIContext::BeginDragDropSource(int flags)
    {
        return ImGui::BeginDragDropSource(static_cast<ImGuiDragDropFlags>(flags));
    }

    bool MteGUIContext::SetDragDropPayload(const std::string& type, uint64_t data)
    {
        return ImGui::SetDragDropPayload(type.c_str(), &data, sizeof(data));
    }

    bool MteGUIContext::SetDragDropPayload(const std::string& type, const std::string& data)
    {
        return ImGui::SetDragDropPayload(type.c_str(), data.c_str(), data.size() + 1);
    }

    void MteGUIContext::EndDragDropSource()
    {
        ImGui::EndDragDropSource();
    }

    bool MteGUIContext::BeginDragDropTarget()
    {
        return ImGui::BeginDragDropTarget();
    }

    bool MteGUIContext::AcceptDragDropPayload(const std::string& type, uint64_t* outData)
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.c_str());
        if (payload && payload->DataSize == sizeof(uint64_t)) {
            *outData = *static_cast<const uint64_t*>(payload->Data);
            return true;
        }
        return false;
    }

    bool MteGUIContext::AcceptDragDropPayload(const std::string& type, std::string* outData)
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type.c_str());
        if (payload && payload->DataSize > 0) {
            *outData = std::string(static_cast<const char*>(payload->Data), payload->DataSize - 1);
            return true;
        }
        return false;
    }

    bool MteGUIContext::AcceptAnyDragDropPayload(std::string* outType, uint64_t* outU64, std::string* outStr,
        bool* outIsU64)
    {
        const ImGuiPayload* preview = ImGui::GetDragDropPayload();
        if (!preview || preview->DataType[0] == '\0')
            return false;
        const ImGuiPayload* acc = ImGui::AcceptDragDropPayload(preview->DataType);
        if (!acc)
            return false;
        outType->assign(preview->DataType);
        if (acc->DataSize == sizeof(uint64_t)) {
            *outIsU64 = true;
            *outU64 = *reinterpret_cast<const uint64_t*>(acc->Data);
            return true;
        }
        *outIsU64 = false;
        if (acc->DataSize > 0) {
            *outStr = std::string(static_cast<const char*>(acc->Data), acc->DataSize - 1);
            return true;
        }
        return false;
    }

    void MteGUIContext::EndDragDropTarget()
    {
        ImGui::EndDragDropTarget();
    }

    void MteGUIContext::SetMouseCursor(int cursorType)
    {
        ImGui::SetMouseCursor(static_cast<ImGuiMouseCursor>(cursorType));
    }

    // ========================================================================
    // Scene View Input API implementation
    // ========================================================================

    bool MteGUIContext::IsMouseButtonDown(int button)
    {
        return ImGui::IsMouseDown(button);
    }

    bool MteGUIContext::IsMouseButtonClicked(int button)
    {
        return ImGui::IsMouseClicked(button);
    }

    bool MteGUIContext::IsMouseDoubleClicked(int button)
    {
        return ImGui::IsMouseDoubleClicked(button);
    }

    bool MteGUIContext::IsMouseDragging(int button, float lockThreshold)
    {
        return ImGui::IsMouseDragging(button, lockThreshold);
    }

    float MteGUIContext::GetMouseDragDeltaX(int button)
    {
        // Use 0.0f threshold for immediate response (no lock threshold)
        return ImGui::GetMouseDragDelta(button, 0.0f).x;
    }

    float MteGUIContext::GetMousePosX()
    {
        return ImGui::GetMousePos().x;
    }

    float MteGUIContext::GetMousePosY()
    {
        return ImGui::GetMousePos().y;
    }

    float MteGUIContext::GetMouseWheelDelta()
    {
        return ImGui::GetIO().MouseWheel;
    }

    bool MteGUIContext::IsKeyDown(int keyCode)
    {
        return ImGui::IsKeyDown(static_cast<ImGuiKey>(keyCode));
    }

    bool MteGUIContext::IsKeyPressed(int keyCode)
    {
        return ImGui::IsKeyPressed(static_cast<ImGuiKey>(keyCode));
    }

    bool MteGUIContext::IsKeyReleased(int keyCode)
    {
        return ImGui::IsKeyReleased(static_cast<ImGuiKey>(keyCode));
    }

    bool MteGUIContext::IsWindowFocused(int flags)
    {
        return ImGui::IsWindowFocused(flags);
    }

    bool MteGUIContext::IsWindowHovered(int flags)
    {
        return ImGui::IsWindowHovered(flags);
    }

    bool MteGUIContext::WantTextInput()
    {
        return ImGui::GetIO().WantTextInput;
    }

    void MteGUIContext::CaptureMouseFromApp(bool capture)
    {
        ImGui::GetIO().WantCaptureMouse = capture;
    }

    void MteGUIContext::CaptureKeyboardFromApp(bool capture)
    {
        ImGui::GetIO().WantCaptureKeyboard = capture;
    }

    void MteGUIContext::GetMainViewportBounds(float* x, float* y, float* w, float* h)
    {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        *x = vp->Pos.x;
        *y = vp->Pos.y;
        *w = vp->Size.x;
        *h = vp->Size.y;
    }

    void MteGUIContext::SetClipboardText(const std::string& text)
    {
        ImGui::SetClipboardText(text.c_str());
    }

    std::string MteGUIContext::GetClipboardText()
    {
        const char* t = ImGui::GetClipboardText();
        return t ? std::string(t) : std::string();
    }

    void MteGUIContext::InputTextMultiline(const std::string& label, const std::string& text, float width, float height,
        int flags)
    {
        // We use a temporary buffer so ImGui can render selection / scrolling,
        // but the content is effectively read-only (ImGuiInputTextFlags_ReadOnly = 1 << 14).
        std::vector<char> buf(text.size() + 1, 0);
        std::copy(text.begin(), text.end(), buf.begin());
        ImGui::InputTextMultiline(label.c_str(), buf.data(), buf.size(), ImVec2(width, height), flags);
    }

    void MteGUIContext::SetWindowFontScale(float scale)
    {
        ImGui::SetWindowFontScale(scale);
    }


    // ==========================================================================
    // Infinite drag — warp cursor to opposite screen edge when it hits a
    // boundary, giving a Unity-style infinite-drag feel for DragFloat etc.
    // Does NOT use SDL relative mouse mode (that conflicts with ImGui).
    // ==========================================================================

    void MteGUIContext::HandleDragCapture()
    {
        const bool active = ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left);

        if (active && !m_dragCaptured) {
            // Just started dragging
            m_dragCaptured = true;
        }

        if (active) {
            // Warp cursor to opposite edge when hitting screen boundary
            POINT mousePos;
            GetCursorPos(&mousePos);
            float mx = static_cast<float>(mousePos.x);
            float my = static_cast<float>(mousePos.y);

            // 获取主显示器边界
            HMONITOR hMonitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(hMonitor, &monitorInfo)) {
                RECT bounds = monitorInfo.rcMonitor;
                const float margin = 2.0f;
                const float left = static_cast<float>(bounds.left) + margin;
                const float right = static_cast<float>(bounds.right) - margin;
                const float top = static_cast<float>(bounds.top) + margin;
                const float bottom = static_cast<float>(bounds.bottom) - margin;

                float newMx = mx, newMy = my;
                bool warped = false;
                if (mx <= left) {
                    newMx = right - margin;
                    warped = true;
                }
                else if (mx >= right) {
                    newMx = left + margin;
                    warped = true;
                }
                if (my <= top) {
                    newMy = bottom - margin;
                    warped = true;
                }
                else if (my >= bottom) {
                    newMy = top + margin;
                    warped = true;
                }

                if (warped) {
                    SetCursorPos(static_cast<int>(newMx), static_cast<int>(newMy));
                    // Ignore two frames of mouse delta after warp to prevent
                    // artificial value jumps in DragFloat / DragInt.
                    // (Some backends report the warp jump on the next frame,
                    // others one frame later.)
                    m_ignoreMouseDeltaFrames = 2;
                }
            }
        }
        else if (m_dragCaptured) {
            // Drag ended — do NOT snap cursor back to start position.
            // Snap-back causes visible stutter and introduces artificial deltas.
            m_dragCaptured = false;
        }
    }

    void MteGUIContext::CompensateWarp()
    {
        ImGuiIO& io = ImGui::GetIO();

        if (m_ignoreMouseDeltaFrames > 0) {
            // Drop delta right after warp (teleport artifact).
            io.MouseDelta = ImVec2(0.0f, 0.0f);
            --m_ignoreMouseDeltaFrames;
            return;
        }

        // Safety net (same idea as Scene panel): ignore implausibly large deltas
        // during drag, which are almost always edge-wrap teleport artifacts.
        constexpr float kWarpJumpThreshold = 400.0f;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
            (std::fabs(io.MouseDelta.x) > kWarpJumpThreshold || std::fabs(io.MouseDelta.y) > kWarpJumpThreshold)) {
            io.MouseDelta = ImVec2(0.0f, 0.0f);
        }
    }

    void MteGUIContext::DrawRect(float minX, float minY, float maxX, float maxY, float r, float g, float b, float a,
        float thickness, float rounding)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), col, rounding, 0, thickness);
        }
    }

    void MteGUIContext::DrawFilledRect(float minX, float minY, float maxX, float maxY, float r, float g, float b, float a,
        float rounding)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->AddRectFilled(ImVec2(minX, minY), ImVec2(maxX, maxY), col, rounding);
        }
    }

    void MteGUIContext::DrawFilledRectRotated(float minX, float minY, float maxX, float maxY, float r, float g, float b,
        float a, float rotation, bool mirrorH, bool mirrorV, float rounding)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList)
            return;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
        const int vtxStart = drawList->VtxBuffer.Size;
        drawList->AddRectFilled(ImVec2(minX, minY), ImVec2(maxX, maxY), col, rounding);

        rotation = std::fmod(rotation, 360.0f);
        if (rotation < 0.0f)
            rotation += 360.0f;
        if ((std::fabs(rotation) < 0.001f) && !mirrorH && !mirrorV)
            return;

        const float radians = rotation * 3.14159265358979f / 180.0f;
        const float cosA = std::cos(radians);
        const float sinA = std::sin(radians);
        const ImVec2 pivot((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            ImVec2 local(drawList->VtxBuffer[i].pos.x - pivot.x, drawList->VtxBuffer[i].pos.y - pivot.y);
            if (mirrorH)
                local.x = -local.x;
            if (mirrorV)
                local.y = -local.y;
            const float rx = local.x * cosA - local.y * sinA;
            const float ry = local.x * sinA + local.y * cosA;
            drawList->VtxBuffer[i].pos = ImVec2(pivot.x + rx, pivot.y + ry);
        }
    }

    void MteGUIContext::DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a,
        float thickness)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, thickness);
        }
    }

    void MteGUIContext::DrawCircle(float centerX, float centerY, float radius, float r, float g, float b, float a,
        float thickness, int segments)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->AddCircle(ImVec2(centerX, centerY), radius, col, segments, thickness);
        }
    }

    void MteGUIContext::DrawFilledCircle(float centerX, float centerY, float radius, float r, float g, float b, float a,
        int segments)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList) {
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->AddCircleFilled(ImVec2(centerX, centerY), radius, col, segments);
        }
    }

    void MteGUIContext::DrawImageRect(uint64_t textureId, float minX, float minY, float maxX, float maxY, float uv0_x,
        float uv0_y, float uv1_x, float uv1_y, float tintR, float tintG, float tintB,
        float tintA, float rotation, bool mirrorH, bool mirrorV, float rounding)
    {
        if (textureId == 0)
            return;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList)
            return;
        ImU32 tint = ImGui::ColorConvertFloat4ToU32(ImVec4(tintR, tintG, tintB, tintA));
        const int vtxStart = drawList->VtxBuffer.Size;
        if (rounding > 0.5f)
            drawList->AddImageRounded(ToImTextureID(textureId), ImVec2(minX, minY), ImVec2(maxX, maxY),
                ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y), tint, rounding);
        else
            drawList->AddImage(ToImTextureID(textureId), ImVec2(minX, minY), ImVec2(maxX, maxY), ImVec2(uv0_x, uv0_y),
                ImVec2(uv1_x, uv1_y), tint);

        rotation = std::fmod(rotation, 360.0f);
        if (rotation < 0.0f)
            rotation += 360.0f;
        if ((std::fabs(rotation) < 0.001f) && !mirrorH && !mirrorV)
            return;

        const float radians = rotation * 3.14159265358979f / 180.0f;
        const float cosA = std::cos(radians);
        const float sinA = std::sin(radians);
        const ImVec2 pivot((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            ImVec2 local(drawList->VtxBuffer[i].pos.x - pivot.x, drawList->VtxBuffer[i].pos.y - pivot.y);
            if (mirrorH)
                local.x = -local.x;
            if (mirrorV)
                local.y = -local.y;
            const float rx = local.x * cosA - local.y * sinA;
            const float ry = local.x * sinA + local.y * cosA;
            drawList->VtxBuffer[i].pos = ImVec2(pivot.x + rx, pivot.y + ry);
        }
    }

    void MteGUIContext::DrawTextDefault(float x, float y, const std::string& text, float r, float g, float b, float a,
        float fontSize)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList)
            return;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, "", ResolveFontSize(fontSize), 0.0f, 1.0f, 0.0f });
        if (!layout.lines.empty()) {
            drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
            textlayout::RenderLine(drawList, layout, layout.lines.front(), x, y, col, 0.0f);
            drawList->PopTextureID();
        }
    }

    void MteGUIContext::DrawTextAligned(float width, float height, const std::string& text, float r,
        float g, float b, float a, float alignX, float alignY, float fontSize, bool clip) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        DrawTextAlignedPos(pos.x, pos.y, pos.x + width, pos.y + height, text, r,g,b,a, alignX, alignY, fontSize, clip);
        ImGui::Dummy(ImVec2(width, height));
    }

    void MteGUIContext::DrawTextAlignedPos(float minX, float minY, float maxX, float maxY, const std::string& text, float r,
        float g, float b, float a, float alignX, float alignY, float fontSize, bool clip)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList)
            return;

        float boxW = maxX - minX;
        float boxH = maxY - minY;

        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, "", ResolveFontSize(fontSize), boxW, 1.0f, 0.0f });

        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));

        if (clip)
            drawList->PushClipRect(ImVec2(minX, minY), ImVec2(maxX, maxY), true);

        drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
        textlayout::RenderTextBox(drawList, minX, minY, maxX, maxY, layout, col, alignX, alignY, 0.0f);
        drawList->PopTextureID();

        if (clip)
            drawList->PopClipRect();
    }

    void MteGUIContext::DrawTextRotated90Aligned(float minX, float minY, float maxX, float maxY, const std::string& text,
        float r, float g, float b, float a, float alignX, float alignY,
        float fontSize, bool clockwise, bool clip)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList || text.empty())
            return;

        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, "", ResolveFontSize(fontSize), 0.0f, 1.0f, 0.0f });
        const ImVec2 textSize(layout.totalWidth, layout.totalHeight);

        float rotatedW = textSize.y;
        float rotatedH = textSize.x;
        float targetX = minX + (maxX - minX - rotatedW) * alignX;
        float targetY = minY + (maxY - minY - rotatedH) * alignY;

        const int vtxStart = drawList->VtxBuffer.Size;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));

        if (clip)
            drawList->PushClipRect(ImVec2(minX, minY), ImVec2(maxX, maxY), true);

        drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
        textlayout::RenderTextBox(drawList, minX, minY, minX + layout.totalWidth, minY + layout.totalHeight, layout, col,
            0.0f, 0.0f, 0.0f);
        drawList->PopTextureID();

        if (clip)
            drawList->PopClipRect();

        if (drawList->VtxBuffer.Size <= vtxStart)
            return;

        ImVec2 boundsMin(FLT_MAX, FLT_MAX);
        ImVec2 boundsMax(-FLT_MAX, -FLT_MAX);
        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            const ImVec2& p = drawList->VtxBuffer[i].pos;
            boundsMin.x = min(boundsMin.x, p.x);
            boundsMin.y = min(boundsMin.y, p.y);
            boundsMax.x = max(boundsMax.x, p.x);
            boundsMax.y = max(boundsMax.y, p.y);
        }

        ImVec2 rotatedMin(FLT_MAX, FLT_MAX);
        ImVec2 rotatedMax(-FLT_MAX, -FLT_MAX);
        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            ImVec2 local(drawList->VtxBuffer[i].pos.x - boundsMin.x, drawList->VtxBuffer[i].pos.y - boundsMin.y);
            ImVec2 rotated = clockwise ? ImVec2(local.y, textSize.x - local.x) : ImVec2(textSize.y - local.y, local.x);
            drawList->VtxBuffer[i].pos = rotated;
            rotatedMin.x = min(rotatedMin.x, rotated.x);
            rotatedMin.y = min(rotatedMin.y, rotated.y);
            rotatedMax.x = max(rotatedMax.x, rotated.x);
            rotatedMax.y = max(rotatedMax.y, rotated.y);
        }

        ImVec2 delta(targetX - rotatedMin.x, targetY - rotatedMin.y);
        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            drawList->VtxBuffer[i].pos.x += delta.x;
            drawList->VtxBuffer[i].pos.y += delta.y;
        }
    }

    void MteGUIContext::DrawTextExAligned(float minX, float minY, float maxX, float maxY, const std::string& text, float r,
        float g, float b, float a, float alignX, float alignY, float fontSize,
        float wrapWidth, float rotation, bool mirrorH, bool mirrorV, bool clip,
        const std::string& fontPath, float lineHeight, float letterSpacing)
    {
        // Normalise rotation to [0, 360)
        rotation = std::fmod(rotation, 360.0f);
        if (rotation < 0.0f)
            rotation += 360.0f;

        // General path: layout the text inside the element box first, then transform
        // the generated vertices around the box center. This keeps editor and game
        // rendering aligned with the component's bounding box semantics.
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (!drawList || text.empty())
            return;

        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, fontPath, ResolveFontSize(fontSize), wrapWidth, lineHeight, letterSpacing });
        const ImVec2 textSize(layout.totalWidth, layout.totalHeight);

        if (std::fabs(rotation) < 0.001f && !mirrorH && !mirrorV) {
            if (clip)
                drawList->PushClipRect(ImVec2(minX, minY), ImVec2(maxX, maxY), true);
            const ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
            drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
            textlayout::RenderTextBox(drawList, minX, minY, maxX, maxY, layout, col, alignX, alignY, letterSpacing);
            drawList->PopTextureID();
            if (clip)
                drawList->PopClipRect();
            return;
        }

        float boxW = maxX - minX;
        float boxH = maxY - minY;
        const ImVec2 pivot((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);

        if (clip)
            drawList->PushClipRect(ImVec2(minX, minY), ImVec2(maxX, maxY), true);

        const int vtxStart = drawList->VtxBuffer.Size;
        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
        drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
        textlayout::RenderTextBox(drawList, minX, minY, maxX, maxY, layout, col, alignX, alignY, letterSpacing);
        drawList->PopTextureID();

        if (clip)
            drawList->PopClipRect();

        if (drawList->VtxBuffer.Size <= vtxStart)
            return;

        // Compute sin/cos for arbitrary angle
        float radians = rotation * 3.14159265358979f / 180.0f;
        float cosA = std::cos(radians);
        float sinA = std::sin(radians);

        for (int i = vtxStart; i < drawList->VtxBuffer.Size; ++i) {
            ImVec2 local(drawList->VtxBuffer[i].pos.x - pivot.x, drawList->VtxBuffer[i].pos.y - pivot.y);
            if (mirrorH)
                local.x = -local.x;
            if (mirrorV)
                local.y = -local.y;
            float rx = local.x * cosA - local.y * sinA;
            float ry = local.x * sinA + local.y * cosA;
            drawList->VtxBuffer[i].pos = ImVec2(pivot.x + rx, pivot.y + ry);
        }
    }

    std::pair<float, float> MteGUIContext::CalcTextSizeA(const std::string& text, float fontSize,
        const std::string& fontPath, float lineHeight, float letterSpacing)
    {
        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, fontPath, ResolveFontSize(fontSize), 0.0f, lineHeight, letterSpacing });
        return { layout.totalWidth, layout.totalHeight };
    }

    std::pair<float, float> MteGUIContext::CalcTextSizeWrappedA(const std::string& text, float fontSize, float wrapWidth,
        const std::string& fontPath, float lineHeight,
        float letterSpacing)
    {
        const textlayout::TextLayoutResult layout =
            textlayout::LayoutText({ text, fontPath, ResolveFontSize(fontSize), wrapWidth, lineHeight, letterSpacing });
        return { layout.totalWidth, layout.totalHeight };
    }

    void MteGUIContext::PushDrawListClipRect(float minX, float minY, float maxX, float maxY, bool intersectWithCurrent)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList)
            drawList->PushClipRect(ImVec2(minX, minY), ImVec2(maxX, maxY), intersectWithCurrent);
    }

    void MteGUIContext::PopDrawListClipRect()
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (drawList)
            drawList->PopClipRect();
    }
} // namespace RenderUI