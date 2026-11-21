// Dear ImGui implementation - see LICENSE.txt.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_internal.h"
#include "imgui_core_private.h"

static const float WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER    = 0.70f;    // Lock scrolled window (so it doesn't pick child windows that are scrolling through) for a certain time, unless mouse moved.

// Pass in translated ASCII characters for text input.
// - with glfw you can get those from the callback set in glfwSetCharCallback()
// - on Windows you can get those using ToAscii+keyboard state, or via the WM_CHAR message
// FIXME: Should in theory be called "AddCharacterEvent()" to be consistent with new API
void ImGuiIO::AddInputCharacter(unsigned int c)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    if (c == 0 || !AppAcceptingEvents)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Text;
    e.Source = ImGuiInputSource_Keyboard;
    e.EventId = g.InputEventsNextEventId++;
    e.Text.Char = c;
    g.InputEventsQueue.push_back(e);
}

// UTF16 strings use surrogate pairs to encode codepoints >= 0x10000, so
// we should save the high surrogate.
void ImGuiIO::AddInputCharacterUTF16(ImWchar16 c)
{
    if ((c == 0 && InputQueueSurrogate == 0) || !AppAcceptingEvents)
        return;

    if ((c & 0xFC00) == 0xD800) // High surrogate, must save
    {
        if (InputQueueSurrogate != 0)
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        InputQueueSurrogate = c;
        return;
    }

    ImWchar cp = c;
    if (InputQueueSurrogate != 0)
    {
        if ((c & 0xFC00) != 0xDC00) // Invalid low surrogate
        {
            AddInputCharacter(IM_UNICODE_CODEPOINT_INVALID);
        }
        else
        {
#if IM_UNICODE_CODEPOINT_MAX == 0xFFFF
            cp = IM_UNICODE_CODEPOINT_INVALID; // Codepoint will not fit in ImWchar
#else
            cp = (ImWchar)(((InputQueueSurrogate - 0xD800) << 10) + (c - 0xDC00) + 0x10000);
#endif
        }

        InputQueueSurrogate = 0;
    }
    AddInputCharacter((unsigned)cp);
}

void ImGuiIO::AddInputCharactersUTF8(const char* str)
{
    if (!AppAcceptingEvents)
        return;
    const char* str_end = str + strlen(str);
    while (*str != 0)
    {
        unsigned int c = 0;
        str += ImTextCharFromUtf8(&c, str, str_end);
        AddInputCharacter(c);
    }
}

// Clear all incoming events.
void ImGuiIO::ClearEventsQueue()
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    g.InputEventsQueue.clear();
}

// Clear current keyboard/gamepad state + current frame text input buffer. Equivalent to releasing all keys/buttons.
void ImGuiIO::ClearInputKeys()
{
    ImGuiContext& g = *Ctx;
    for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
    {
        if (ImGui::IsMouseKey((ImGuiKey)key))
            continue;
        ImGuiKeyData* key_data = &g.IO.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->Down = false;
        key_data->DownDuration = -1.0f;
        key_data->DownDurationPrev = -1.0f;
    }
    KeyCtrl = KeyShift = KeyAlt = KeySuper = false;
    KeyMods = ImGuiMod_None;
    InputQueueCharacters.resize(0); // Behavior of old ClearInputCharacters().
}

void ImGuiIO::ClearInputMouse()
{
    for (ImGuiKey key = ImGuiKey_Mouse_BEGIN; key < ImGuiKey_Mouse_END; key = (ImGuiKey)(key + 1))
    {
        ImGuiKeyData* key_data = &KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->Down = false;
        key_data->DownDuration = -1.0f;
        key_data->DownDurationPrev = -1.0f;
    }
    MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    for (int n = 0; n < IM_COUNTOF(MouseDown); n++)
    {
        MouseDown[n] = false;
        MouseDownDuration[n] = MouseDownDurationPrev[n] = -1.0f;
    }
    MouseWheel = MouseWheelH = 0.0f;
}

static ImGuiInputEvent* FindLatestInputEvent(ImGuiContext* ctx, ImGuiInputEventType type, int arg = -1)
{
    ImGuiContext& g = *ctx;
    for (int n = g.InputEventsQueue.Size - 1; n >= 0; n--)
    {
        ImGuiInputEvent* e = &g.InputEventsQueue[n];
        if (e->Type != type)
            continue;
        if (type == ImGuiInputEventType_Key && e->Key.Key != arg)
            continue;
        if (type == ImGuiInputEventType_MouseButton && e->MouseButton.Button != arg)
            continue;
        return e;
    }
    return NULL;
}

// Queue a new key down/up event.
// - ImGuiKey key:       Translated key (as in, generally ImGuiKey_A matches the key end-user would use to emit an 'A' character)
// - bool down:          Is the key down? use false to signify a key release.
// - float analog_value: 0.0f..1.0f
// IMPORTANT: THIS FUNCTION AND OTHER "ADD" GRABS THE CONTEXT FROM OUR INSTANCE.
// WE NEED TO ENSURE THAT ALL FUNCTION CALLS ARE FULFILLING THIS, WHICH IS WHY GetKeyData() HAS AN EXPLICIT CONTEXT.
void ImGuiIO::AddKeyAnalogEvent(ImGuiKey key, bool down, float analog_value)
{
    //if (e->Down) { IMGUI_DEBUG_LOG_IO("AddKeyEvent() Key='%s' %d, NativeKeycode = %d, NativeScancode = %d\n", ImGui::GetKeyName(e->Key), e->Down, e->NativeKeycode, e->NativeScancode); }
    IM_ASSERT(Ctx != NULL);
    if (key == ImGuiKey_None || !AppAcceptingEvents)
        return;
    ImGuiContext& g = *Ctx;
    IM_ASSERT(ImGui::IsNamedKeyOrMod(key)); // Backend needs to pass a valid ImGuiKey_ constant. 0..511 values are legacy native key codes which are not accepted by this API.
    IM_ASSERT(ImGui::IsAliasKey(key) == false); // Backend cannot submit ImGuiKey_MouseXXX values they are automatically inferred from AddMouseXXX() events.

    // MacOS: swap Cmd(Super) and Ctrl
    if (g.IO.ConfigMacOSXBehaviors)
    {
        if (key == ImGuiMod_Super)          { key = ImGuiMod_Ctrl; }
        else if (key == ImGuiMod_Ctrl)      { key = ImGuiMod_Super; }
        else if (key == ImGuiKey_LeftSuper) { key = ImGuiKey_LeftCtrl; }
        else if (key == ImGuiKey_RightSuper){ key = ImGuiKey_RightCtrl; }
        else if (key == ImGuiKey_LeftCtrl)  { key = ImGuiKey_LeftSuper; }
        else if (key == ImGuiKey_RightCtrl) { key = ImGuiKey_RightSuper; }
    }

    // Filter duplicate (in particular: key mods and gamepad analog values are commonly spammed)
    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_Key, (int)key);
    const ImGuiKeyData* key_data = ImGui::GetKeyData(&g, key);
    const bool latest_key_down = latest_event ? latest_event->Key.Down : key_data->Down;
    const float latest_key_analog = latest_event ? latest_event->Key.AnalogValue : key_data->AnalogValue;
    if (latest_key_down == down && latest_key_analog == analog_value)
        return;

    // Add event
    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Key;
    e.Source = ImGui::IsGamepadKey(key) ? ImGuiInputSource_Gamepad : ImGuiInputSource_Keyboard;
    e.EventId = g.InputEventsNextEventId++;
    e.Key.Key = key;
    e.Key.Down = down;
    e.Key.AnalogValue = analog_value;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddKeyEvent(ImGuiKey key, bool down)
{
    if (!AppAcceptingEvents)
        return;
    AddKeyAnalogEvent(key, down, down ? 1.0f : 0.0f);
}

// [Optional] Call after AddKeyEvent().
// Specify native keycode, scancode + Specify index for legacy <1.87 IsKeyXXX() functions with native indices.
// If you are writing a backend in 2022 or don't use IsKeyXXX() with native values that are not ImGuiKey values, you can avoid calling this.
void ImGuiIO::SetKeyEventNativeData(ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index)
{
    if (key == ImGuiKey_None)
        return;
    IM_ASSERT(ImGui::IsNamedKey(key)); // >= 512
    IM_ASSERT(native_legacy_index == -1 || ImGui::IsLegacyKey((ImGuiKey)native_legacy_index)); // >= 0 && <= 511
    IM_UNUSED(key);                 // Yet unused
    IM_UNUSED(native_keycode);      // Yet unused
    IM_UNUSED(native_scancode);     // Yet unused
    IM_UNUSED(native_legacy_index); // Yet unused
}

// Set master flag for accepting key/mouse/text events (default to true). Useful if you have native dialog boxes that are interrupting your application loop/refresh, and you want to disable events being queued while your app is frozen.
void ImGuiIO::SetAppAcceptingEvents(bool accepting_events)
{
    AppAcceptingEvents = accepting_events;
}

// Queue a mouse move event
void ImGuiIO::AddMousePosEvent(float x, float y)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    if (!AppAcceptingEvents)
        return;

    // Apply same flooring as UpdateMouseInputs()
    ImVec2 pos((x > -FLT_MAX) ? ImFloor(x) : x, (y > -FLT_MAX) ? ImFloor(y) : y);

    // Filter duplicate
    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_MousePos);
    const ImVec2 latest_pos = latest_event ? ImVec2(latest_event->MousePos.PosX, latest_event->MousePos.PosY) : g.IO.MousePos;
    if (latest_pos.x == pos.x && latest_pos.y == pos.y)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MousePos;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MousePos.PosX = pos.x;
    e.MousePos.PosY = pos.y;
    e.MousePos.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddMouseButtonEvent(int mouse_button, bool down)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    IM_ASSERT(mouse_button >= 0 && mouse_button < ImGuiMouseButton_COUNT);
    if (!AppAcceptingEvents)
        return;

    // On MacOS X: Convert Ctrl(Super)+Left click into Right-click: handle held button.
    if (ConfigMacOSXBehaviors && mouse_button == 0 && MouseCtrlLeftAsRightClick)
    {
        // Order of both statements matters: this event will still release mouse button 1
        mouse_button = 1;
        if (!down)
            MouseCtrlLeftAsRightClick = false;
    }

    // Filter duplicate
    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_MouseButton, (int)mouse_button);
    const bool latest_button_down = latest_event ? latest_event->MouseButton.Down : g.IO.MouseDown[mouse_button];
    if (latest_button_down == down)
        return;

    // On MacOS X: Convert Ctrl(Super)+Left click into Right-click.
    // - Note that this is actual physical Ctrl which is ImGuiMod_Super for us.
    // - At this point we want from !down to down, so this is handling the initial press.
    if (ConfigMacOSXBehaviors && mouse_button == 0 && down)
    {
        const ImGuiInputEvent* latest_super_event = FindLatestInputEvent(&g, ImGuiInputEventType_Key, (int)ImGuiMod_Super);
        if (latest_super_event ? latest_super_event->Key.Down : g.IO.KeySuper)
        {
            IMGUI_DEBUG_LOG_IO("[io] Super+Left Click aliased into Right Click\n");
            MouseCtrlLeftAsRightClick = true;
            AddMouseButtonEvent(1, true); // This is just quicker to write that passing through, as we need to filter duplicate again.
            return;
        }
    }

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MouseButton;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MouseButton.Button = mouse_button;
    e.MouseButton.Down = down;
    e.MouseButton.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

// Queue a mouse wheel event (some mouse/API may only have a Y component)
void ImGuiIO::AddMouseWheelEvent(float wheel_x, float wheel_y)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;

    // Filter duplicate (unlike most events, wheel values are relative and easy to filter)
    if (!AppAcceptingEvents || (wheel_x == 0.0f && wheel_y == 0.0f))
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MouseWheel;
    e.Source = ImGuiInputSource_Mouse;
    e.EventId = g.InputEventsNextEventId++;
    e.MouseWheel.WheelX = wheel_x;
    e.MouseWheel.WheelY = wheel_y;
    e.MouseWheel.MouseSource = g.InputEventsNextMouseSource;
    g.InputEventsQueue.push_back(e);
}

// This is not a real event, the data is latched in order to be stored in actual Mouse events.
// This is so that duplicate events (e.g. Windows sending extraneous WM_MOUSEMOVE) gets filtered and are not leading to actual source changes.
void ImGuiIO::AddMouseSourceEvent(ImGuiMouseSource source)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    g.InputEventsNextMouseSource = source;
}

void ImGuiIO::AddMouseViewportEvent(ImGuiID viewport_id)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;
    //IM_ASSERT(g.IO.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport);
    if (!AppAcceptingEvents)
        return;

    // Filter duplicate
    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_MouseViewport);
    const ImGuiID latest_viewport_id = latest_event ? latest_event->MouseViewport.HoveredViewportID : g.IO.MouseHoveredViewport;
    if (latest_viewport_id == viewport_id)
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_MouseViewport;
    e.Source = ImGuiInputSource_Mouse;
    e.MouseViewport.HoveredViewportID = viewport_id;
    g.InputEventsQueue.push_back(e);
}

void ImGuiIO::AddFocusEvent(bool focused)
{
    IM_ASSERT(Ctx != NULL);
    ImGuiContext& g = *Ctx;

    // Filter duplicate
    const ImGuiInputEvent* latest_event = FindLatestInputEvent(&g, ImGuiInputEventType_Focus);
    const bool latest_focused = latest_event ? latest_event->AppFocused.Focused : !g.IO.AppFocusLost;
    if (latest_focused == focused || (ConfigDebugIgnoreFocusLoss && !focused))
        return;

    ImGuiInputEvent e;
    e.Type = ImGuiInputEventType_Focus;
    e.EventId = g.InputEventsNextEventId++;
    e.AppFocused.Focused = focused;
    g.InputEventsQueue.push_back(e);
}

//-----------------------------------------------------------------------------
// [SECTION] INPUTS
//-----------------------------------------------------------------------------
// - GetModForLRModKey() [Internal]
// - FixupKeyChord() [Internal]
// - GetKeyData() [Internal]
// - GetKeyIndex() [Internal]
// - GetKeyName()
// - GetKeyChordName() [Internal]
// - CalcTypematicRepeatAmount() [Internal]
// - GetTypematicRepeatRate() [Internal]
// - GetKeyPressedAmount() [Internal]
// - GetKeyMagnitude2d() [Internal]
//-----------------------------------------------------------------------------
// - UpdateKeyRoutingTable() [Internal]
// - GetRoutingIdFromOwnerId() [Internal]
// - GetShortcutRoutingData() [Internal]
// - CalcRoutingScore() [Internal]
// - SetShortcutRouting() [Internal]
// - TestShortcutRouting() [Internal]
//-----------------------------------------------------------------------------
// - IsKeyDown()
// - IsKeyPressed()
// - IsKeyReleased()
//-----------------------------------------------------------------------------
// - IsMouseDown()
// - IsMouseClicked()
// - IsMouseReleased()
// - IsMouseDoubleClicked()
// - GetMouseClickedCount()
// - IsMouseHoveringRect() [Internal]
// - IsMouseDragPastThreshold() [Internal]
// - IsMouseDragging()
// - GetMousePos()
// - SetMousePos() [Internal]
// - GetMousePosOnOpeningCurrentPopup()
// - IsMousePosValid()
// - IsAnyMouseDown()
// - GetMouseDragDelta()
// - ResetMouseDragDelta()
// - GetMouseCursor()
// - SetMouseCursor()
//-----------------------------------------------------------------------------
// - UpdateAliasKey()
// - GetMergedModsFromKeys()
// - UpdateKeyboardInputs()
// - UpdateMouseInputs()
//-----------------------------------------------------------------------------
// - LockWheelingWindow [Internal]
// - FindBestWheelingWindow [Internal]
// - UpdateMouseWheel() [Internal]
//-----------------------------------------------------------------------------
// - SetNextFrameWantCaptureKeyboard()
// - SetNextFrameWantCaptureMouse()
//-----------------------------------------------------------------------------
// - GetInputSourceName() [Internal]
// - DebugPrintInputEvent() [Internal]
// - UpdateInputEvents() [Internal]
//-----------------------------------------------------------------------------
// - GetKeyOwner() [Internal]
// - TestKeyOwner() [Internal]
// - SetKeyOwner() [Internal]
// - SetItemKeyOwner() [Internal]
// - Shortcut() [Internal]
//-----------------------------------------------------------------------------

static ImGuiKeyChord GetModForLRModKey(ImGuiKey key)
{
    if (key == ImGuiKey_LeftCtrl || key == ImGuiKey_RightCtrl)
        return ImGuiMod_Ctrl;
    if (key == ImGuiKey_LeftShift || key == ImGuiKey_RightShift)
        return ImGuiMod_Shift;
    if (key == ImGuiKey_LeftAlt || key == ImGuiKey_RightAlt)
        return ImGuiMod_Alt;
    if (key == ImGuiKey_LeftSuper || key == ImGuiKey_RightSuper)
        return ImGuiMod_Super;
    return ImGuiMod_None;
}

ImGuiKeyChord ImGui::FixupKeyChord(ImGuiKeyChord key_chord)
{
    // Add ImGuiMod_XXXX when a corresponding ImGuiKey_LeftXXX/ImGuiKey_RightXXX is specified.
    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (IsLRModKey(key))
        key_chord |= GetModForLRModKey(key);
    return key_chord;
}

ImGuiKeyData* ImGui::GetKeyData(ImGuiContext* ctx, ImGuiKey key)
{
    ImGuiContext& g = *ctx;

    // Special storage location for mods
    if (key & ImGuiMod_Mask_)
        key = ConvertSingleModFlagToKey(key);

    IM_ASSERT(IsNamedKey(key) && "Support for user key indices was dropped in favor of ImGuiKey. Please update backend & user code.");
    return &g.IO.KeysData[key - ImGuiKey_NamedKey_BEGIN];
}

// Those names are provided for debugging purpose and are not meant to be saved persistently nor compared.
static const char* const GKeyNames[] =
{
    "Tab", "LeftArrow", "RightArrow", "UpArrow", "DownArrow", "PageUp", "PageDown",
    "Home", "End", "Insert", "Delete", "Backspace", "Space", "Enter", "Escape",
    "LeftCtrl", "LeftShift", "LeftAlt", "LeftSuper", "RightCtrl", "RightShift", "RightAlt", "RightSuper", "Menu",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24",
    "Apostrophe", "Comma", "Minus", "Period", "Slash", "Semicolon", "Equal", "LeftBracket",
    "Backslash", "RightBracket", "GraveAccent", "CapsLock", "ScrollLock", "NumLock", "PrintScreen",
    "Pause", "Keypad0", "Keypad1", "Keypad2", "Keypad3", "Keypad4", "Keypad5", "Keypad6",
    "Keypad7", "Keypad8", "Keypad9", "KeypadDecimal", "KeypadDivide", "KeypadMultiply",
    "KeypadSubtract", "KeypadAdd", "KeypadEnter", "KeypadEqual",
    "AppBack", "AppForward", "Oem102",
    "GamepadStart", "GamepadBack",
    "GamepadFaceLeft", "GamepadFaceRight", "GamepadFaceUp", "GamepadFaceDown",
    "GamepadDpadLeft", "GamepadDpadRight", "GamepadDpadUp", "GamepadDpadDown",
    "GamepadL1", "GamepadR1", "GamepadL2", "GamepadR2", "GamepadL3", "GamepadR3",
    "GamepadLStickLeft", "GamepadLStickRight", "GamepadLStickUp", "GamepadLStickDown",
    "GamepadRStickLeft", "GamepadRStickRight", "GamepadRStickUp", "GamepadRStickDown",
    "MouseLeft", "MouseRight", "MouseMiddle", "MouseX1", "MouseX2", "MouseWheelX", "MouseWheelY",
    "ModCtrl", "ModShift", "ModAlt", "ModSuper", // ReservedForModXXX are showing the ModXXX names.
};
IM_STATIC_ASSERT(ImGuiKey_NamedKey_COUNT == IM_COUNTOF(GKeyNames));

const char* ImGui::GetKeyName(ImGuiKey key)
{
    if (key == ImGuiKey_None)
        return "None";
    IM_ASSERT(IsNamedKeyOrMod(key) && "Support for user key indices was dropped in favor of ImGuiKey. Please update backend and user code.");
    if (key & ImGuiMod_Mask_)
        key = ConvertSingleModFlagToKey(key);
    if (!IsNamedKey(key))
        return "Unknown";

    return GKeyNames[key - ImGuiKey_NamedKey_BEGIN];
}

// Return untranslated names: on macOS, Cmd key will show as Ctrl, Ctrl key will show as super.
// Lifetime of return value: valid until next call to same function.
const char* ImGui::GetKeyChordName(ImGuiKeyChord key_chord)
{
    ImGuiContext& g = *GImGui;

    const ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (IsLRModKey(key))
        key_chord &= ~GetModForLRModKey(key); // Return "Ctrl+LeftShift" instead of "Ctrl+Shift+LeftShift"
    ImFormatString(g.TempKeychordName, IM_COUNTOF(g.TempKeychordName), "%s%s%s%s%s",
        (key_chord & ImGuiMod_Ctrl) ? "Ctrl+" : "",
        (key_chord & ImGuiMod_Shift) ? "Shift+" : "",
        (key_chord & ImGuiMod_Alt) ? "Alt+" : "",
        (key_chord & ImGuiMod_Super) ? "Super+" : "",
        (key != ImGuiKey_None || key_chord == ImGuiKey_None) ? GetKeyName(key) : "");
    size_t len;
    if (key == ImGuiKey_None && key_chord != 0)
        if ((len = ImStrlen(g.TempKeychordName)) != 0) // Remove trailing '+'
            g.TempKeychordName[len - 1] = 0;
    return g.TempKeychordName;
}

// t0 = previous time (e.g.: g.Time - g.IO.DeltaTime)
// t1 = current time (e.g.: g.Time)
// An event is triggered at:
//  t = 0.0f     t = repeat_delay,    t = repeat_delay + repeat_rate*N
int ImGui::CalcTypematicRepeatAmount(float t0, float t1, float repeat_delay, float repeat_rate)
{
    if (t1 == 0.0f)
        return 1;
    if (t0 >= t1)
        return 0;
    if (repeat_rate <= 0.0f)
        return t0 < repeat_delay && t1 >= repeat_delay;
    const int count_t0 = (t0 < repeat_delay) ? -1 : (int)((t0 - repeat_delay) / repeat_rate);
    const int count_t1 = (t1 < repeat_delay) ? -1 : (int)((t1 - repeat_delay) / repeat_rate);
    const int count = count_t1 - count_t0;
    return count;
}

void ImGui::GetTypematicRepeatRate(ImGuiInputFlags flags, float* repeat_delay, float* repeat_rate)
{
    ImGuiContext& g = *GImGui;
    switch (flags & ImGuiInputFlags_RepeatRateMask_)
    {
    case ImGuiInputFlags_RepeatRateNavMove:             *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.80f; return;
    case ImGuiInputFlags_RepeatRateNavTweak:            *repeat_delay = g.IO.KeyRepeatDelay * 0.72f; *repeat_rate = g.IO.KeyRepeatRate * 0.30f; return;
    case ImGuiInputFlags_RepeatRateDefault: default:    *repeat_delay = g.IO.KeyRepeatDelay * 1.00f; *repeat_rate = g.IO.KeyRepeatRate * 1.00f; return;
    }
}

// Return value representing the number of presses in the last time period, for the given repeat rate
// (most often returns 0 or 1. The result is generally only >1 when RepeatRate is smaller than DeltaTime, aka large DeltaTime or fast RepeatRate)
int ImGui::GetKeyPressedAmount(ImGuiKey key, float repeat_delay, float repeat_rate)
{
    ImGuiContext& g = *GImGui;
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
        return 0;
    const float t = key_data->DownDuration;
    return CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, repeat_delay, repeat_rate);
}

// Return 2D vector representing the combination of four cardinal direction, with analog value support (for e.g. ImGuiKey_GamepadLStick* values).
ImVec2 ImGui::GetKeyMagnitude2d(ImGuiKey key_left, ImGuiKey key_right, ImGuiKey key_up, ImGuiKey key_down)
{
    return ImVec2(
        GetKeyData(key_right)->AnalogValue - GetKeyData(key_left)->AnalogValue,
        GetKeyData(key_down)->AnalogValue - GetKeyData(key_up)->AnalogValue);
}

// Rewrite routing data buffers to strip old entries + sort by key to make queries not touch scattered data.
//   Entries   D,A,B,B,A,C,B     --> A,A,B,B,B,C,D
//   Index     A:1 B:2 C:5 D:0   --> A:0 B:2 C:5 D:6
// See 'Metrics->Key Owners & Shortcut Routing' to visualize the result of that operation.
static void UpdateKeyRoutingTable(ImGuiKeyRoutingTable* rt)
{
    ImGuiContext& g = *GImGui;
    rt->EntriesNext.resize(0);
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
    {
        const int new_routing_start_idx = rt->EntriesNext.Size;
        ImGuiKeyRoutingData* routing_entry;
        for (int old_routing_idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; old_routing_idx != -1; old_routing_idx = routing_entry->NextEntryIndex)
        {
            routing_entry = &rt->Entries[old_routing_idx];
            routing_entry->RoutingCurrScore = routing_entry->RoutingNextScore;
            routing_entry->RoutingCurr = routing_entry->RoutingNext; // Update entry
            routing_entry->RoutingNext = ImGuiKeyOwner_NoOwner;
            routing_entry->RoutingNextScore = 0;
            if (routing_entry->RoutingCurr == ImGuiKeyOwner_NoOwner)
                continue;
            rt->EntriesNext.push_back(*routing_entry); // Write alive ones into new buffer

            // Apply routing to owner if there's no owner already (RoutingCurr == None at this point)
            // This is the result of previous frame's SetShortcutRouting() call.
            if (routing_entry->Mods == g.IO.KeyMods)
            {
                ImGuiKeyOwnerData* owner_data = ImGui::GetKeyOwnerData(&g, key);
                if (owner_data->OwnerCurr == ImGuiKeyOwner_NoOwner)
                {
                    owner_data->OwnerCurr = routing_entry->RoutingCurr;
                    //IMGUI_DEBUG_LOG("SetKeyOwner(%s, owner_id=0x%08X) via Routing\n", GetKeyName(key), routing_entry->RoutingCurr);
                }
            }
        }

        // Rewrite linked-list
        rt->Index[key - ImGuiKey_NamedKey_BEGIN] = (ImGuiKeyRoutingIndex)(new_routing_start_idx < rt->EntriesNext.Size ? new_routing_start_idx : -1);
        for (int n = new_routing_start_idx; n < rt->EntriesNext.Size; n++)
            rt->EntriesNext[n].NextEntryIndex = (ImGuiKeyRoutingIndex)((n + 1 < rt->EntriesNext.Size) ? n + 1 : -1);
    }
    rt->Entries.swap(rt->EntriesNext); // Swap new and old indexes
}

// owner_id may be None/Any, but routing_id needs to be always be set, so we default to GetCurrentFocusScope().
static inline ImGuiID GetRoutingIdFromOwnerId(ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    return (owner_id != ImGuiKeyOwner_NoOwner && owner_id != ImGuiKeyOwner_Any) ? owner_id : g.CurrentFocusScopeId;
}

ImGuiKeyRoutingData* ImGui::GetShortcutRoutingData(ImGuiKeyChord key_chord)
{
    // Majority of shortcuts will be Key + any number of Mods
    // We accept _Single_ mod with ImGuiKey_None.
    //  - Shortcut(ImGuiKey_S | ImGuiMod_Ctrl);                    // Legal
    //  - Shortcut(ImGuiKey_S | ImGuiMod_Ctrl | ImGuiMod_Shift);   // Legal
    //  - Shortcut(ImGuiMod_Ctrl);                                 // Legal
    //  - Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift);                // Not legal
    ImGuiContext& g = *GImGui;
    ImGuiKeyRoutingTable* rt = &g.KeysRoutingTable;
    ImGuiKeyRoutingData* routing_data;
    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        key = ConvertSingleModFlagToKey(mods);
    IM_ASSERT(IsNamedKey(key));

    // Get (in the majority of case, the linked list will have one element so this should be 2 reads.
    // Subsequent elements will be contiguous in memory as list is sorted/rebuilt in NewFrame).
    for (ImGuiKeyRoutingIndex idx = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; idx != -1; idx = routing_data->NextEntryIndex)
    {
        routing_data = &rt->Entries[idx];
        if (routing_data->Mods == mods)
            return routing_data;
    }

    // Add to linked-list
    ImGuiKeyRoutingIndex routing_data_idx = (ImGuiKeyRoutingIndex)rt->Entries.Size;
    rt->Entries.push_back(ImGuiKeyRoutingData());
    routing_data = &rt->Entries[routing_data_idx];
    routing_data->Mods = (ImU16)mods;
    routing_data->NextEntryIndex = rt->Index[key - ImGuiKey_NamedKey_BEGIN]; // Setup linked list
    rt->Index[key - ImGuiKey_NamedKey_BEGIN] = routing_data_idx;
    return routing_data;
}

// Current score encoding
//  -        0: Never route
//  -        1: ImGuiInputFlags_RouteGlobal    (lower priority)
//  - 100..199: ImGuiInputFlags_RouteFocused   (if window in focus-stack)
//         200: ImGuiInputFlags_RouteGlobal  | ImGuiInputFlags_RouteOverFocused
//         300: ImGuiInputFlags_RouteActive or ImGuiInputFlags_RouteFocused (if item active)
//         400: ImGuiInputFlags_RouteGlobal  | ImGuiInputFlags_RouteOverActive
//  - 500..599: ImGuiInputFlags_RouteFocused | ImGuiInputFlags_RouteOverActive (if window in focus-stack) (higher priority)
// 'flags' should include an explicit routing policy
static int CalcRoutingScore(ImGuiID focus_scope_id, ImGuiID owner_id, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (flags & ImGuiInputFlags_RouteFocused)
    {
        // ActiveID gets high priority
        // (we don't check g.ActiveIdUsingAllKeys here. Routing is applied but if input ownership is tested later it may discard it)
        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;

        // Score based on distance to focused window (lower is better)
        // Assuming both windows are submitting a routing request,
        // - When Window....... is focused -> Window scores 3 (best), Window/ChildB scores 255 (no match)
        // - When Window/ChildB is focused -> Window scores 4,        Window/ChildB scores 3 (best)
        // Assuming only WindowA is submitting a routing request,
        // - When Window/ChildB is focused -> Window scores 4 (best), Window/ChildB doesn't have a score.
        // This essentially follow the window->ParentWindowForFocusRoute chain.
        if (focus_scope_id == 0)
            return 0;
        for (int index_in_focus_path = 0; index_in_focus_path < g.NavFocusRoute.Size; index_in_focus_path++)
            if (g.NavFocusRoute.Data[index_in_focus_path].ID == focus_scope_id)
            {
                if (flags & ImGuiInputFlags_RouteOverActive) // && g.ActiveId != 0 && g.ActiveId != owner_id)
                    return 599 - index_in_focus_path;
                else
                    return 199 - index_in_focus_path;
            }
        return 0;
    }
    else if (flags & ImGuiInputFlags_RouteActive)
    {
        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;
        return 0;
    }
    else if (flags & ImGuiInputFlags_RouteGlobal)
    {
        if (flags & ImGuiInputFlags_RouteOverActive)
            return 400;
        if (owner_id != 0 && g.ActiveId == owner_id)
            return 300;
        if (flags & ImGuiInputFlags_RouteOverFocused)
            return 200;
        return 1;
    }
    IM_ASSERT(0);
    return 0;
}

// - We need this to filter some Shortcut() routes when an item e.g. an InputText() is active
//   e.g. ImGuiKey_G won't be considered a shortcut when item is active, but ImGuiMod|ImGuiKey_G can be.
// - This is also used by UpdateInputEvents() to avoid trickling in the most common case of e.g. pressing ImGuiKey_G also emitting a G character.
static bool IsKeyChordPotentiallyCharInput(ImGuiKeyChord key_chord)
{
    // Mimic 'ignore_char_inputs' logic in InputText()
    ImGuiContext& g = *GImGui;

    // When the right mods are pressed it cannot be a char input so we won't filter the shortcut out.
    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    const bool ignore_char_inputs = ((mods & ImGuiMod_Ctrl) && !(mods & ImGuiMod_Alt)) || (g.IO.ConfigMacOSXBehaviors && (mods & ImGuiMod_Ctrl));
    if (ignore_char_inputs)
        return false;

    // Return true for A-Z, 0-9 and other keys associated to char inputs. Other keys such as F1-F12 won't be filtered.
    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        return false;
    return g.KeysMayBeCharInput.TestBit(key);
}

// Request a desired route for an input chord (key + mods).
// Return true if the route is available this frame.
// - Routes and key ownership are attributed at the beginning of next frame based on best score and mod state.
//   (Conceptually this does a "Submit for next frame" + "Test for current frame".
//   As such, it could be called TrySetXXX or SubmitXXX, or the Submit and Test operations should be separate.)
bool ImGui::SetShortcutRouting(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    if ((flags & ImGuiInputFlags_RouteTypeMask_) == 0)
        flags |= ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteOverActive; // IMPORTANT: This is the default for SetShortcutRouting() but NOT Shortcut()
    else
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiInputFlags_RouteTypeMask_)); // Check that only 1 routing flag is used
    IM_ASSERT(owner_id != ImGuiKeyOwner_Any && owner_id != ImGuiKeyOwner_NoOwner);
    if (flags & (ImGuiInputFlags_RouteOverFocused | ImGuiInputFlags_RouteUnlessBgFocused))
        IM_ASSERT(flags & ImGuiInputFlags_RouteGlobal);
    if (flags & ImGuiInputFlags_RouteOverActive)
        IM_ASSERT(flags & (ImGuiInputFlags_RouteGlobal | ImGuiInputFlags_RouteFocused));

    // Add ImGuiMod_XXXX when a corresponding ImGuiKey_LeftXXX/ImGuiKey_RightXXX is specified.
    key_chord = FixupKeyChord(key_chord);

    // [DEBUG] Debug break requested by user
    if (g.DebugBreakInShortcutRouting == key_chord)
        IM_DEBUG_BREAK();

    if (flags & ImGuiInputFlags_RouteUnlessBgFocused)
        if (g.NavWindow == NULL)
            return false;

    // Note how ImGuiInputFlags_RouteAlways won't set routing and thus won't set owner. May want to rework this?
    if (flags & ImGuiInputFlags_RouteAlways)
    {
        IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> always, no register\n", GetKeyChordName(key_chord), flags, owner_id);
        return true;
    }

    // Specific culling when there's an active item.
    if (g.ActiveId != 0 && g.ActiveId != owner_id)
    {
        if (flags & ImGuiInputFlags_RouteActive)
            return false;

        // Cull shortcuts with no modifiers when it could generate a character.
        // e.g. Shortcut(ImGuiKey_G) also generates 'g' character, should not trigger when InputText() is active.
        // but  Shortcut(Ctrl+G) should generally trigger when InputText() is active.
        // TL;DR: lettered shortcut with no mods or with only Alt mod will not trigger while an item reading text input is active.
        // (We cannot filter based on io.InputQueueCharacters[] contents because of trickling and key<>chars submission order are undefined)
        if (g.IO.WantTextInput && IsKeyChordPotentiallyCharInput(key_chord))
        {
            IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> filtered as potential char input\n", GetKeyChordName(key_chord), flags, owner_id);
            return false;
        }

        // ActiveIdUsingAllKeyboardKeys trumps all for ActiveId
        if ((flags & ImGuiInputFlags_RouteOverActive) == 0 && g.ActiveIdUsingAllKeyboardKeys)
        {
            ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
            if (key == ImGuiKey_None)
                key = ConvertSingleModFlagToKey((ImGuiKey)(key_chord & ImGuiMod_Mask_));
            if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
                return false;
        }
    }

    // Where do we evaluate route for?
    ImGuiID focus_scope_id = g.CurrentFocusScopeId;
    if (flags & ImGuiInputFlags_RouteFromRootWindow)
        focus_scope_id = g.CurrentWindow->RootWindow->ID; // See PushFocusScope() call in Begin()

    const int score = CalcRoutingScore(focus_scope_id, owner_id, flags);
    IMGUI_DEBUG_LOG_INPUTROUTING("SetShortcutRouting(%s, flags=%04X, owner_id=0x%08X) -> score %d\n", GetKeyChordName(key_chord), flags, owner_id, score);
    if (score == 0)
        return false;

    // Submit routing for NEXT frame (assuming score is sufficient)
    // FIXME: Could expose a way to use a "serve last" policy for same score resolution (using >= instead of >).
    ImGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord);
    //const bool set_route = (flags & ImGuiInputFlags_ServeLast) ? (score >= routing_data->RoutingNextScore) : (score > routing_data->RoutingNextScore);
    if (score > routing_data->RoutingNextScore)
    {
        routing_data->RoutingNext = owner_id;
        routing_data->RoutingNextScore = (ImU16)score;
    }

    // Return routing state for CURRENT frame
    if (routing_data->RoutingCurr == owner_id)
        IMGUI_DEBUG_LOG_INPUTROUTING("--> granting current route\n");
    return routing_data->RoutingCurr == owner_id;
}

// Currently unused by core (but used by tests)
// Note: this cannot be turned into GetShortcutRouting() because we do the owner_id->routing_id translation, name would be more misleading.
bool ImGui::TestShortcutRouting(ImGuiKeyChord key_chord, ImGuiID owner_id)
{
    const ImGuiID routing_id = GetRoutingIdFromOwnerId(owner_id);
    key_chord = FixupKeyChord(key_chord);
    ImGuiKeyRoutingData* routing_data = GetShortcutRoutingData(key_chord); // FIXME: Could avoid creating entry.
    return routing_data->RoutingCurr == routing_id;
}

// Note that Dear ImGui doesn't know the meaning/semantic of ImGuiKey from 0..511: they are legacy native keycodes.
// Consider transitioning from 'IsKeyDown(MY_ENGINE_KEY_A)' (<1.87) to IsKeyDown(ImGuiKey_A) (>= 1.87)
bool ImGui::IsKeyDown(ImGuiKey key)
{
    return IsKeyDown(key, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyDown(ImGuiKey key, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsKeyPressed(ImGuiKey key, bool repeat)
{
    return IsKeyPressed(key, repeat ? ImGuiInputFlags_Repeat : ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

// Important: unlike legacy IsKeyPressed(ImGuiKey, bool repeat=true) which DEFAULT to repeat, this requires EXPLICIT repeat.
bool ImGui::IsKeyPressed(ImGuiKey key, ImGuiInputFlags flags, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (!key_data->Down) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
        return false;
    const float t = key_data->DownDuration;
    if (t < 0.0f)
        return false;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByIsKeyPressed) == 0); // Passing flags not supported by this function!
    if (flags & (ImGuiInputFlags_RepeatRateMask_ | ImGuiInputFlags_RepeatUntilMask_)) // Setting any _RepeatXXX option enables _Repeat
        flags |= ImGuiInputFlags_Repeat;

    bool pressed = (t == 0.0f);
    if (!pressed && (flags & ImGuiInputFlags_Repeat) != 0)
    {
        float repeat_delay, repeat_rate;
        GetTypematicRepeatRate(flags, &repeat_delay, &repeat_rate);
        pressed = (t > repeat_delay) && GetKeyPressedAmount(key, repeat_delay, repeat_rate) > 0;
        if (pressed && (flags & ImGuiInputFlags_RepeatUntilMask_))
        {
            // Slightly bias 'key_pressed_time' as DownDuration is an accumulation of DeltaTime which we compare to an absolute time value.
            // Ideally we'd replace DownDuration with KeyPressedTime but it would break user's code.
            ImGuiContext& g = *GImGui;
            double key_pressed_time = g.Time - t + 0.00001f;
            if ((flags & ImGuiInputFlags_RepeatUntilKeyModsChange) && (g.LastKeyModsChangeTime > key_pressed_time))
                pressed = false;
            if ((flags & ImGuiInputFlags_RepeatUntilKeyModsChangeFromNone) && (g.LastKeyModsChangeFromNoneTime > key_pressed_time))
                pressed = false;
            if ((flags & ImGuiInputFlags_RepeatUntilOtherKeyPress) && (g.LastKeyboardKeyPressTime > key_pressed_time))
                pressed = false;
        }
    }
    if (!pressed)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsKeyReleased(ImGuiKey key)
{
    return IsKeyReleased(key, ImGuiKeyOwner_Any);
}

bool ImGui::IsKeyReleased(ImGuiKey key, ImGuiID owner_id)
{
    const ImGuiKeyData* key_data = GetKeyData(key);
    if (key_data->DownDurationPrev < 0.0f || key_data->Down)
        return false;
    if (!TestKeyOwner(key, owner_id))
        return false;
    return true;
}

bool ImGui::IsMouseDown(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseDown[button] && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any); // should be same as IsKeyDown(MouseButtonToKey(button), ImGuiKeyOwner_Any), but this allows legacy code hijacking the io.Mousedown[] array.
}

bool ImGui::IsMouseDown(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseDown[button] && TestKeyOwner(MouseButtonToKey(button), owner_id); // Should be same as IsKeyDown(MouseButtonToKey(button), owner_id), but this allows legacy code hijacking the io.Mousedown[] array.
}

bool ImGui::IsMouseClicked(ImGuiMouseButton button, bool repeat)
{
    return IsMouseClicked(button, repeat ? ImGuiInputFlags_Repeat : ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseClicked(ImGuiMouseButton button, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (!g.IO.MouseDown[button]) // In theory this should already be encoded as (DownDuration < 0.0f), but testing this facilitates eating mechanism (until we finish work on key ownership)
        return false;
    const float t = g.IO.MouseDownDuration[button];
    if (t < 0.0f)
        return false;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByIsMouseClicked) == 0); // Passing flags not supported by this function! // FIXME: Could support RepeatRate and RepeatUntil flags here.

    const bool repeat = (flags & ImGuiInputFlags_Repeat) != 0;
    const bool pressed = (t == 0.0f) || (repeat && t > g.IO.KeyRepeatDelay && CalcTypematicRepeatAmount(t - g.IO.DeltaTime, t, g.IO.KeyRepeatDelay, g.IO.KeyRepeatRate) > 0);
    if (!pressed)
        return false;

    if (!TestKeyOwner(MouseButtonToKey(button), owner_id))
        return false;

    return true;
}

bool ImGui::IsMouseReleased(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseReleased[button] && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any); // Should be same as IsKeyReleased(MouseButtonToKey(button), ImGuiKeyOwner_Any)
}

bool ImGui::IsMouseReleased(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseReleased[button] && TestKeyOwner(MouseButtonToKey(button), owner_id); // Should be same as IsKeyReleased(MouseButtonToKey(button), owner_id)
}

// Use if you absolutely need to distinguish single-click from double-click by introducing a delay.
// Generally use with 'delay >= io.MouseDoubleClickTime' + combined with a 'io.MouseClickedLastCount == 1' test.
// This is a very rarely used UI idiom, but some apps use this: e.g. MS Explorer single click on an icon to rename.
bool ImGui::IsMouseReleasedWithDelay(ImGuiMouseButton button, float delay)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    const float time_since_release = (float)(g.Time - g.IO.MouseReleasedTime[button]);
    return !IsMouseDown(button) && (time_since_release - g.IO.DeltaTime < delay) && (time_since_release >= delay);
}

bool ImGui::IsMouseDoubleClicked(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button] == 2 && TestKeyOwner(MouseButtonToKey(button), ImGuiKeyOwner_Any);
}

bool ImGui::IsMouseDoubleClicked(ImGuiMouseButton button, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button] == 2 && TestKeyOwner(MouseButtonToKey(button), owner_id);
}

int ImGui::GetMouseClickedCount(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    return g.IO.MouseClickedCount[button];
}

// Test if mouse cursor is hovering given rectangle
// NB- Rectangle is clipped by our current clip setting
// NB- Expand the rectangle to be generous on imprecise inputs systems (g.Style.TouchExtraPadding)
bool ImGui::IsMouseHoveringRect(const ImVec2& r_min, const ImVec2& r_max, bool clip)
{
    ImGuiContext& g = *GImGui;

    // Clip
    ImRect rect_clipped(r_min, r_max);
    if (clip)
        rect_clipped.ClipWith(g.CurrentWindow->ClipRect);

    // Hit testing, expanded for touch input
    if (!rect_clipped.ContainsWithPad(g.IO.MousePos, g.Style.TouchExtraPadding))
        return false;
    if (!g.MouseViewport->GetMainRect().Overlaps(rect_clipped))
        return false;
    return true;
}

// Return if a mouse click/drag went past the given threshold. Valid to call during the MouseReleased frame.
// [Internal] This doesn't test if the button is pressed
bool ImGui::IsMouseDragPastThreshold(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    return g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold;
}

bool ImGui::IsMouseDragging(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (!g.IO.MouseDown[button])
        return false;
    return IsMouseDragPastThreshold(button, lock_threshold);
}

ImVec2 ImGui::GetMousePos()
{
    ImGuiContext& g = *GImGui;
    return g.IO.MousePos;
}

// This is called TeleportMousePos() and not SetMousePos() to emphasis that setting MousePosPrev will effectively clear mouse delta as well.
// It is expected you only call this if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos) is set and supported by backend.
void ImGui::TeleportMousePos(const ImVec2& pos)
{
    ImGuiContext& g = *GImGui;
    g.IO.MousePos = g.IO.MousePosPrev = pos;
    g.IO.MouseDelta = ImVec2(0.0f, 0.0f);
    g.IO.WantSetMousePos = true;
    //IMGUI_DEBUG_LOG_IO("TeleportMousePos: (%.1f,%.1f)\n", io.MousePos.x, io.MousePos.y);
}

// NB: prefer to call right after BeginPopup(). At the time Selectable/MenuItem is activated, the popup is already closed!
ImVec2 ImGui::GetMousePosOnOpeningCurrentPopup()
{
    ImGuiContext& g = *GImGui;
    if (g.BeginPopupStack.Size > 0)
        return g.OpenPopupStack[g.BeginPopupStack.Size - 1].OpenMousePos;
    return g.IO.MousePos;
}

// We typically use ImVec2(-FLT_MAX,-FLT_MAX) to denote an invalid mouse position.
bool ImGui::IsMousePosValid(const ImVec2* mouse_pos)
{
    // The assert is only to silence a false-positive in XCode Static Analysis.
    // Because GImGui is not dereferenced in every code path, the static analyzer assume that it may be NULL (which it doesn't for other functions).
    IM_ASSERT(GImGui != NULL);
    const float MOUSE_INVALID = -256000.0f;
    ImVec2 p = mouse_pos ? *mouse_pos : GImGui->IO.MousePos;
    return p.x >= MOUSE_INVALID && p.y >= MOUSE_INVALID;
}

// [WILL OBSOLETE] This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
bool ImGui::IsAnyMouseDown()
{
    ImGuiContext& g = *GImGui;
    for (int n = 0; n < IM_COUNTOF(g.IO.MouseDown); n++)
        if (g.IO.MouseDown[n])
            return true;
    return false;
}

// Return the delta from the initial clicking position while the mouse button is clicked or was just released.
// This is locked and return 0.0f until the mouse moves past a distance threshold at least once.
// NB: This is only valid if IsMousePosValid(). backends in theory should always keep mouse position valid when dragging even outside the client window.
ImVec2 ImGui::GetMouseDragDelta(ImGuiMouseButton button, float lock_threshold)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    if (lock_threshold < 0.0f)
        lock_threshold = g.IO.MouseDragThreshold;
    if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
        if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
            if (IsMousePosValid(&g.IO.MousePos) && IsMousePosValid(&g.IO.MouseClickedPos[button]))
                return g.IO.MousePos - g.IO.MouseClickedPos[button];
    return ImVec2(0.0f, 0.0f);
}

void ImGui::ResetMouseDragDelta(ImGuiMouseButton button)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(button >= 0 && button < IM_COUNTOF(g.IO.MouseDown));
    // NB: We don't need to reset g.IO.MouseDragMaxDistanceSqr
    g.IO.MouseClickedPos[button] = g.IO.MousePos;
}

// Get desired mouse cursor shape.
// Important: this is meant to be used by a platform backend, it is reset in ImGui::NewFrame(),
// updated during the frame, and locked in EndFrame()/Render().
// If you use software rendering by setting io.MouseDrawCursor then Dear ImGui will render those for you
ImGuiMouseCursor ImGui::GetMouseCursor()
{
    ImGuiContext& g = *GImGui;
    return g.MouseCursor;
}

// We intentionally accept values of ImGuiMouseCursor that are outside our bounds, in case users needs to hack-in a custom cursor value.
// Custom cursors may be handled by custom backends. If you are using a standard backend and want to hack in a custom cursor, you may
// handle it before the backend _NewFrame() call and temporarily set ImGuiConfigFlags_NoMouseCursorChange during the backend _NewFrame() call.
void ImGui::SetMouseCursor(ImGuiMouseCursor cursor_type)
{
    ImGuiContext& g = *GImGui;
    g.MouseCursor = cursor_type;
}

static void UpdateAliasKey(ImGuiKey key, bool v, float analog_value)
{
    IM_ASSERT(ImGui::IsAliasKey(key));
    ImGuiKeyData* key_data = ImGui::GetKeyData(key);
    key_data->Down = v;
    key_data->AnalogValue = analog_value;
}

// [Internal] Do not use directly
ImGuiKeyChord ImGui::GetMergedModsFromKeys()
{
    // Bypass IsKeyDown() for the unlikely case where user used a ImGuiInputFlags_LockXXXX on those.
    ImGuiKeyChord mods = 0;
    if (ImGui::GetKeyData(ImGuiMod_Ctrl)->Down)     { mods |= ImGuiMod_Ctrl; }
    if (ImGui::GetKeyData(ImGuiMod_Shift)->Down)    { mods |= ImGuiMod_Shift; }
    if (ImGui::GetKeyData(ImGuiMod_Alt)->Down)      { mods |= ImGuiMod_Alt; }
    if (ImGui::GetKeyData(ImGuiMod_Super)->Down)    { mods |= ImGuiMod_Super; }
    return mods;
}

void ImGui::UpdateKeyboardInputs()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
        io.ClearInputKeys();

    // Update aliases
    for (int n = 0; n < ImGuiMouseButton_COUNT; n++)
        UpdateAliasKey(MouseButtonToKey(n), io.MouseDown[n], io.MouseDown[n] ? 1.0f : 0.0f);
    UpdateAliasKey(ImGuiKey_MouseWheelX, io.MouseWheelH != 0.0f, io.MouseWheelH);
    UpdateAliasKey(ImGuiKey_MouseWheelY, io.MouseWheel != 0.0f, io.MouseWheel);

    // Synchronize io.KeyMods and io.KeyCtrl/io.KeyShift/etc. values.
    // - New backends (1.87+): send io.AddKeyEvent(ImGuiMod_XXX) ->                                      -> (here) deriving io.KeyMods + io.KeyXXX from key array.
    // - Legacy backends:      set io.KeyXXX bools               -> (above) set key array from io.KeyXXX -> (here) deriving io.KeyMods + io.KeyXXX from key array.
    // So with legacy backends the 4 values will do a unnecessary back-and-forth but it makes the code simpler and future facing.
    const ImGuiKeyChord prev_key_mods = io.KeyMods;
    io.KeyMods = GetMergedModsFromKeys();
    io.KeyCtrl = (io.KeyMods & ImGuiMod_Ctrl) != 0;
    io.KeyShift = (io.KeyMods & ImGuiMod_Shift) != 0;
    io.KeyAlt = (io.KeyMods & ImGuiMod_Alt) != 0;
    io.KeySuper = (io.KeyMods & ImGuiMod_Super) != 0;
    if (prev_key_mods != io.KeyMods)
        g.LastKeyModsChangeTime = g.Time;
    if (prev_key_mods != io.KeyMods && prev_key_mods == 0)
        g.LastKeyModsChangeFromNoneTime = g.Time;

    // Clear gamepad data if disabled
    if ((io.BackendFlags & ImGuiBackendFlags_HasGamepad) == 0)
        for (int key = ImGuiKey_Gamepad_BEGIN; key < ImGuiKey_Gamepad_END; key++)
        {
            io.KeysData[key - ImGuiKey_NamedKey_BEGIN].Down = false;
            io.KeysData[key - ImGuiKey_NamedKey_BEGIN].AnalogValue = 0.0f;
        }

    // Update keys
    for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
    {
        ImGuiKeyData* key_data = &io.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        key_data->DownDurationPrev = key_data->DownDuration;
        key_data->DownDuration = key_data->Down ? (key_data->DownDuration < 0.0f ? 0.0f : key_data->DownDuration + io.DeltaTime) : -1.0f;
        if (key_data->DownDuration == 0.0f)
        {
            if (IsKeyboardKey((ImGuiKey)key))
                g.LastKeyboardKeyPressTime = g.Time;
            else if (key == ImGuiKey_ReservedForModCtrl || key == ImGuiKey_ReservedForModShift || key == ImGuiKey_ReservedForModAlt || key == ImGuiKey_ReservedForModSuper)
                g.LastKeyboardKeyPressTime = g.Time;
        }
    }

    // Update keys/input owner (named keys only): one entry per key
    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
    {
        ImGuiKeyData* key_data = &io.KeysData[key - ImGuiKey_NamedKey_BEGIN];
        ImGuiKeyOwnerData* owner_data = &g.KeysOwnerData[key - ImGuiKey_NamedKey_BEGIN];
        owner_data->OwnerCurr = owner_data->OwnerNext;
        if (!key_data->Down) // Important: ownership is released on the frame after a release. Ensure a 'MouseDown -> CloseWindow -> MouseUp' chain doesn't lead to someone else seeing the MouseUp.
            owner_data->OwnerNext = ImGuiKeyOwner_NoOwner;
        owner_data->LockThisFrame = owner_data->LockUntilRelease = owner_data->LockUntilRelease && key_data->Down;  // Clear LockUntilRelease when key is not Down anymore
    }

    // Update key routing (for e.g. shortcuts)
    UpdateKeyRoutingTable(&g.KeysRoutingTable);
}

void ImGui::UpdateMouseInputs()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    // Mouse Wheel swapping flag
    // As a standard behavior holding Shift while using Vertical Mouse Wheel triggers Horizontal scroll instead
    // - We avoid doing it on OSX as it the OS input layer handles this already.
    // - FIXME: However this means when running on OSX over Emscripten, Shift+WheelY will incur two swapping (1 in OS, 1 here), canceling the feature.
    // - FIXME: When we can distinguish e.g. touchpad scroll events from mouse ones, we'll set this accordingly based on input source.
    io.MouseWheelRequestAxisSwap = io.KeyShift && !io.ConfigMacOSXBehaviors;

    // Round mouse position to avoid spreading non-rounded position (e.g. UpdateManualResize doesn't support them well)
    if (IsMousePosValid(&io.MousePos))
        io.MousePos = g.MouseLastValidPos = ImFloor(io.MousePos);

    // If mouse just appeared or disappeared (usually denoted by -FLT_MAX components) we cancel out movement in MouseDelta
    if (IsMousePosValid(&io.MousePos) && IsMousePosValid(&io.MousePosPrev))
        io.MouseDelta = io.MousePos - io.MousePosPrev;
    else
        io.MouseDelta = ImVec2(0.0f, 0.0f);

    // Update stationary timer.
    // FIXME: May need to rework again to have some tolerance for occasional small movement, while being functional on high-framerates.
    const float mouse_stationary_threshold = (io.MouseSource == ImGuiMouseSource_Mouse) ? 2.0f : 3.0f; // Slightly higher threshold for ImGuiMouseSource_TouchScreen/ImGuiMouseSource_Pen, may need rework.
    const bool mouse_stationary = (ImLengthSqr(io.MouseDelta) <= mouse_stationary_threshold * mouse_stationary_threshold);
    g.MouseStationaryTimer = mouse_stationary ? (g.MouseStationaryTimer + io.DeltaTime) : 0.0f;
    //IMGUI_DEBUG_LOG("%.4f\n", g.MouseStationaryTimer);

    // If mouse moved we re-enable mouse hovering in case it was disabled by keyboard/gamepad. In theory should use a >0.0f threshold but would need to reset in everywhere we set this to true.
    if (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)
        g.NavHighlightItemUnderNav = false;

    for (int i = 0; i < IM_COUNTOF(io.MouseDown); i++)
    {
        io.MouseClicked[i] = io.MouseDown[i] && io.MouseDownDuration[i] < 0.0f;
        io.MouseClickedCount[i] = 0; // Will be filled below
        io.MouseReleased[i] = !io.MouseDown[i] && io.MouseDownDuration[i] >= 0.0f;
        if (io.MouseReleased[i])
            io.MouseReleasedTime[i] = g.Time;
        io.MouseDownDurationPrev[i] = io.MouseDownDuration[i];
        io.MouseDownDuration[i] = io.MouseDown[i] ? (io.MouseDownDuration[i] < 0.0f ? 0.0f : io.MouseDownDuration[i] + io.DeltaTime) : -1.0f;
        if (io.MouseClicked[i])
        {
            bool is_repeated_click = false;
            if ((float)(g.Time - io.MouseClickedTime[i]) < io.MouseDoubleClickTime)
            {
                ImVec2 delta_from_click_pos = IsMousePosValid(&io.MousePos) ? (io.MousePos - io.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
                if (ImLengthSqr(delta_from_click_pos) < io.MouseDoubleClickMaxDist * io.MouseDoubleClickMaxDist)
                    is_repeated_click = true;
            }
            if (is_repeated_click)
                io.MouseClickedLastCount[i]++;
            else
                io.MouseClickedLastCount[i] = 1;
            io.MouseClickedTime[i] = g.Time;
            io.MouseClickedPos[i] = io.MousePos;
            io.MouseClickedCount[i] = io.MouseClickedLastCount[i];
            io.MouseDragMaxDistanceAbs[i] = ImVec2(0.0f, 0.0f);
            io.MouseDragMaxDistanceSqr[i] = 0.0f;
        }
        else if (io.MouseDown[i])
        {
            // Maintain the maximum distance we reaching from the initial click position, which is used with dragging threshold
            ImVec2 delta_from_click_pos = IsMousePosValid(&io.MousePos) ? (io.MousePos - io.MouseClickedPos[i]) : ImVec2(0.0f, 0.0f);
            io.MouseDragMaxDistanceSqr[i] = ImMax(io.MouseDragMaxDistanceSqr[i], ImLengthSqr(delta_from_click_pos));
            io.MouseDragMaxDistanceAbs[i].x = ImMax(io.MouseDragMaxDistanceAbs[i].x, delta_from_click_pos.x < 0.0f ? -delta_from_click_pos.x : delta_from_click_pos.x);
            io.MouseDragMaxDistanceAbs[i].y = ImMax(io.MouseDragMaxDistanceAbs[i].y, delta_from_click_pos.y < 0.0f ? -delta_from_click_pos.y : delta_from_click_pos.y);
        }

        // We provide io.MouseDoubleClicked[] as a legacy service
        io.MouseDoubleClicked[i] = (io.MouseClickedCount[i] == 2);

        // Clicking any mouse button reactivate mouse hovering which may have been deactivated by keyboard/gamepad navigation
        if (io.MouseClicked[i])
            g.NavHighlightItemUnderNav = false;
    }
}

static void LockWheelingWindow(ImGuiWindow* window, float wheel_amount)
{
    ImGuiContext& g = *GImGui;
    if (window)
        g.WheelingWindowReleaseTimer = ImMin(g.WheelingWindowReleaseTimer + ImAbs(wheel_amount) * WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER, WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER);
    else
        g.WheelingWindowReleaseTimer = 0.0f;
    if (g.WheelingWindow == window)
        return;
    IMGUI_DEBUG_LOG_IO("[io] LockWheelingWindow() \"%s\"\n", window ? window->Name : "NULL");
    g.WheelingWindow = window;
    g.WheelingWindowRefMousePos = g.IO.MousePos;
    if (window == NULL)
    {
        g.WheelingWindowStartFrame = -1;
        g.WheelingAxisAvg = ImVec2(0.0f, 0.0f);
    }
}

static ImGuiWindow* FindBestWheelingWindow(const ImVec2& wheel)
{
    // For each axis, find window in the hierarchy that may want to use scrolling
    ImGuiContext& g = *GImGui;
    ImGuiWindow* windows[2] = { NULL, NULL };
    for (int axis = 0; axis < 2; axis++)
        if (wheel[axis] != 0.0f)
            for (ImGuiWindow* window = windows[axis] = g.HoveredWindow; window->Flags & ImGuiWindowFlags_ChildWindow; window = windows[axis] = window->ParentWindow)
            {
                // Bubble up into parent window if:
                // - a child window doesn't allow any scrolling.
                // - a child window has the ImGuiWindowFlags_NoScrollWithMouse flag.
                //// - a child window doesn't need scrolling because it is already at the edge for the direction we are going in (FIXME-WIP)
                const bool has_scrolling = (window->ScrollMax[axis] != 0.0f);
                const bool inputs_disabled = (window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs);
                //const bool scrolling_past_limits = (wheel_v < 0.0f) ? (window->Scroll[axis] <= 0.0f) : (window->Scroll[axis] >= window->ScrollMax[axis]);
                if (has_scrolling && !inputs_disabled) // && !scrolling_past_limits)
                    break; // select this window
            }
    if (windows[0] == NULL && windows[1] == NULL)
        return NULL;

    // If there's only one window or only one axis then there's no ambiguity
    if (windows[0] == windows[1] || windows[0] == NULL || windows[1] == NULL)
        return windows[1] ? windows[1] : windows[0];

    // If candidate are different windows we need to decide which one to prioritize
    // - First frame: only find a winner if one axis is zero.
    // - Subsequent frames: only find a winner when one is more than the other.
    if (g.WheelingWindowStartFrame == -1)
        g.WheelingWindowStartFrame = g.FrameCount;
    if ((g.WheelingWindowStartFrame == g.FrameCount && wheel.x != 0.0f && wheel.y != 0.0f) || (g.WheelingAxisAvg.x == g.WheelingAxisAvg.y))
    {
        g.WheelingWindowWheelRemainder = wheel;
        return NULL;
    }
    return (g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? windows[0] : windows[1];
}

// Called by NewFrame()
void ImGui::UpdateMouseWheel()
{
    // Reset the locked window if we move the mouse or after the timer elapses.
    // FIXME: Ideally we could refactor to have one timer for "changing window w/ same axis" and a shorter timer for "changing window or axis w/ other axis" (#3795)
    ImGuiContext& g = *GImGui;
    if (g.WheelingWindow != NULL)
    {
        g.WheelingWindowReleaseTimer -= g.IO.DeltaTime;
        if (IsMousePosValid() && ImLengthSqr(g.IO.MousePos - g.WheelingWindowRefMousePos) > g.IO.MouseDragThreshold * g.IO.MouseDragThreshold)
            g.WheelingWindowReleaseTimer = 0.0f;
        if (g.WheelingWindowReleaseTimer <= 0.0f)
            LockWheelingWindow(NULL, 0.0f);
    }

    ImVec2 wheel;
    wheel.x = TestKeyOwner(ImGuiKey_MouseWheelX, ImGuiKeyOwner_NoOwner) ? g.IO.MouseWheelH : 0.0f;
    wheel.y = TestKeyOwner(ImGuiKey_MouseWheelY, ImGuiKeyOwner_NoOwner) ? g.IO.MouseWheel : 0.0f;

    //IMGUI_DEBUG_LOG("MouseWheel X:%.3f Y:%.3f\n", wheel_x, wheel_y);
    ImGuiWindow* mouse_window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;
    if (!mouse_window || mouse_window->Collapsed)
        return;

    // Zoom / Scale window
    // FIXME-OBSOLETE: This is an old feature, it still works but pretty much nobody is using it and may be best redesigned.
    if (wheel.y != 0.0f && g.IO.KeyCtrl && g.IO.FontAllowUserScaling)
    {
        LockWheelingWindow(mouse_window, wheel.y);
        ImGuiWindow* window = mouse_window;
        const float new_font_scale = ImClamp(window->FontWindowScale + g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
        const float scale = new_font_scale / window->FontWindowScale;
        window->FontWindowScale = new_font_scale;
        if (window == window->RootWindow)
        {
            const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
            SetWindowPos(window, window->Pos + offset, 0);
            window->Size = ImTrunc(window->Size * scale); // FIXME: Legacy-ish code, call SetWindowSize()?
            window->SizeFull = ImTrunc(window->SizeFull * scale);
            MarkIniSettingsDirty(window);
        }
        return;
    }
    if (g.IO.KeyCtrl)
        return;

    // Mouse wheel scrolling
    // Read about io.MouseWheelRequestAxisSwap and its issue on Mac+Emscripten in UpdateMouseInputs()
    if (g.IO.MouseWheelRequestAxisSwap)
        wheel = ImVec2(wheel.y, 0.0f);

    // Maintain a rough average of moving magnitude on both axes
    // FIXME: should by based on wall clock time rather than frame-counter
    g.WheelingAxisAvg.x = ImExponentialMovingAverage(g.WheelingAxisAvg.x, ImAbs(wheel.x), 30);
    g.WheelingAxisAvg.y = ImExponentialMovingAverage(g.WheelingAxisAvg.y, ImAbs(wheel.y), 30);

    // In the rare situation where FindBestWheelingWindow() had to defer first frame of wheeling due to ambiguous main axis, reinject it now.
    wheel += g.WheelingWindowWheelRemainder;
    g.WheelingWindowWheelRemainder = ImVec2(0.0f, 0.0f);
    if (wheel.x == 0.0f && wheel.y == 0.0f)
        return;

    // Mouse wheel scrolling: find target and apply
    // - don't renew lock if axis doesn't apply on the window.
    // - select a main axis when both axes are being moved.
    if (ImGuiWindow* window = (g.WheelingWindow ? g.WheelingWindow : FindBestWheelingWindow(wheel)))
        if (!(window->Flags & ImGuiWindowFlags_NoScrollWithMouse) && !(window->Flags & ImGuiWindowFlags_NoMouseInputs))
        {
            bool do_scroll[2] = { wheel.x != 0.0f && window->ScrollMax.x != 0.0f, wheel.y != 0.0f && window->ScrollMax.y != 0.0f };
            if (do_scroll[ImGuiAxis_X] && do_scroll[ImGuiAxis_Y])
                do_scroll[(g.WheelingAxisAvg.x > g.WheelingAxisAvg.y) ? ImGuiAxis_Y : ImGuiAxis_X] = false;
            if (do_scroll[ImGuiAxis_X])
            {
                LockWheelingWindow(window, wheel.x);
                float max_step = window->InnerRect.GetWidth() * 0.67f;
                float scroll_step = ImTrunc(ImMin(2 * window->FontRefSize, max_step));
                SetScrollX(window, window->Scroll.x - wheel.x * scroll_step);
                g.WheelingWindowScrolledFrame = g.FrameCount;
            }
            if (do_scroll[ImGuiAxis_Y])
            {
                LockWheelingWindow(window, wheel.y);
                float max_step = window->InnerRect.GetHeight() * 0.67f;
                float scroll_step = ImTrunc(ImMin(5 * window->FontRefSize, max_step));
                SetScrollY(window, window->Scroll.y - wheel.y * scroll_step);
                g.WheelingWindowScrolledFrame = g.FrameCount;
            }
        }
}

void ImGui::SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard)
{
    ImGuiContext& g = *GImGui;
    g.WantCaptureKeyboardNextFrame = want_capture_keyboard ? 1 : 0;
}

void ImGui::SetNextFrameWantCaptureMouse(bool want_capture_mouse)
{
    ImGuiContext& g = *GImGui;
    g.WantCaptureMouseNextFrame = want_capture_mouse ? 1 : 0;
}


// Process input queue
// We always call this with the value of 'bool g.IO.ConfigInputTrickleEventQueue'.
// - trickle_fast_inputs = false : process all events, turn into flattened input state (e.g. successive down/up/down/up will be lost)
// - trickle_fast_inputs = true  : process as many events as possible (successive down/up/down/up will be trickled over several frames so nothing is lost) (new feature in 1.87)
void ImGui::UpdateInputEvents(bool trickle_fast_inputs)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    // Only trickle chars<>key when working with InputText()
    // FIXME: InputText() could parse event trail?
    // FIXME: Could specialize chars<>keys trickling rules for control keys (those not typically associated to characters)
    const bool trickle_interleaved_nonchar_keys_and_text = (trickle_fast_inputs && g.WantTextInputNextFrame == 1);

    bool mouse_moved = false, mouse_wheeled = false, key_changed = false, key_changed_nonchar = false, text_inputted = false;
    int  mouse_button_changed = 0x00;
    ImBitArray<ImGuiKey_NamedKey_COUNT> key_changed_mask;

    int event_n = 0;
    for (; event_n < g.InputEventsQueue.Size; event_n++)
    {
        ImGuiInputEvent* e = &g.InputEventsQueue[event_n];
        if (e->Type == ImGuiInputEventType_MousePos)
        {
            if (g.IO.WantSetMousePos)
                continue;
            // Trickling Rule: Stop processing queued events if we already handled a mouse button change
            ImVec2 event_pos(e->MousePos.PosX, e->MousePos.PosY);
            if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_wheeled || key_changed || text_inputted))
                break;
            io.MousePos = event_pos;
            io.MouseSource = e->MousePos.MouseSource;
            mouse_moved = true;
        }
        else if (e->Type == ImGuiInputEventType_MouseButton)
        {
            // Trickling Rule: Stop processing queued events if we got multiple action on the same button
            const ImGuiMouseButton button = e->MouseButton.Button;
            IM_ASSERT(button >= 0 && button < ImGuiMouseButton_COUNT);
            if (trickle_fast_inputs && ((mouse_button_changed & (1 << button)) || mouse_wheeled))
                break;
            if (trickle_fast_inputs && e->MouseButton.MouseSource == ImGuiMouseSource_TouchScreen && mouse_moved) // #2702: TouchScreen have no initial hover.
                break;
            io.MouseDown[button] = e->MouseButton.Down;
            io.MouseSource = e->MouseButton.MouseSource;
            mouse_button_changed |= (1 << button);
        }
        else if (e->Type == ImGuiInputEventType_MouseWheel)
        {
            // Trickling Rule: Stop processing queued events if we got multiple action on the event
            if (trickle_fast_inputs && (mouse_moved || mouse_button_changed != 0))
                break;
            io.MouseWheelH += e->MouseWheel.WheelX;
            io.MouseWheel += e->MouseWheel.WheelY;
            io.MouseSource = e->MouseWheel.MouseSource;
            mouse_wheeled = true;
        }
        else if (e->Type == ImGuiInputEventType_MouseViewport)
        {
            io.MouseHoveredViewport = e->MouseViewport.HoveredViewportID;
        }
        else if (e->Type == ImGuiInputEventType_Key)
        {
            // Trickling Rule: Stop processing queued events if we got multiple action on the same button
            if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
                continue;
            ImGuiKey key = e->Key.Key;
            IM_ASSERT(key != ImGuiKey_None);
            ImGuiKeyData* key_data = GetKeyData(key);
            const int key_data_index = (int)(key_data - g.IO.KeysData);
            if (trickle_fast_inputs && key_data->Down != e->Key.Down && (key_changed_mask.TestBit(key_data_index) || mouse_button_changed != 0))
                break;

            const bool key_is_potentially_for_char_input = IsKeyChordPotentiallyCharInput(GetMergedModsFromKeys() | key);
            if (trickle_interleaved_nonchar_keys_and_text && (text_inputted && !key_is_potentially_for_char_input))
                break;

            if (key_data->Down != e->Key.Down) // Analog change only do not trigger this, so it won't block e.g. further mouse pos events testing key_changed.
            {
                key_changed = true;
                key_changed_mask.SetBit(key_data_index);
                if (trickle_interleaved_nonchar_keys_and_text && !key_is_potentially_for_char_input)
                    key_changed_nonchar = true;
            }

            key_data->Down = e->Key.Down;
            key_data->AnalogValue = e->Key.AnalogValue;
        }
        else if (e->Type == ImGuiInputEventType_Text)
        {
            if (io.ConfigFlags & ImGuiConfigFlags_NoKeyboard)
                continue;
            // Trickling Rule: Stop processing queued events if keys/mouse have been interacted with
            if (trickle_fast_inputs && (mouse_button_changed != 0 || mouse_moved || mouse_wheeled))
                break;
            if (trickle_interleaved_nonchar_keys_and_text && key_changed_nonchar)
                break;
            unsigned int c = e->Text.Char;
            io.InputQueueCharacters.push_back(c <= IM_UNICODE_CODEPOINT_MAX ? (ImWchar)c : IM_UNICODE_CODEPOINT_INVALID);
            if (trickle_interleaved_nonchar_keys_and_text)
                text_inputted = true;
        }
        else if (e->Type == ImGuiInputEventType_Focus)
        {
            // We intentionally overwrite this and process in NewFrame(), in order to give a chance
            // to multi-viewports backends to queue AddFocusEvent(false) + AddFocusEvent(true) in same frame.
            const bool focus_lost = !e->AppFocused.Focused;
            io.AppFocusLost = focus_lost;
        }
        else
        {
            IM_ASSERT(0 && "Unknown event!");
        }
    }

    // Record trail (for domain-specific applications wanting to access a precise trail)
    //if (event_n != 0) IMGUI_DEBUG_LOG_IO("Processed: %d / Remaining: %d\n", event_n, g.InputEventsQueue.Size - event_n);
    for (int n = 0; n < event_n; n++)
        g.InputEventsTrail.push_back(g.InputEventsQueue[n]);

    // [DEBUG]
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
    if (event_n != 0 && (g.DebugLogFlags & ImGuiDebugLogFlags_EventIO))
        for (int n = 0; n < g.InputEventsQueue.Size; n++)
            DebugPrintInputEvent(n < event_n ? "Processed" : "Remaining", &g.InputEventsQueue[n]);
#endif

    // Remaining events will be processed on the next frame
    // FIXME-MULTITHREADING: io.AddKeyEvent() etc. calls are mostly thread-safe apart from the fact they push to this
    // queue which may be resized here. Could potentially rework this to narrow down the section needing a mutex? (#5772)
    if (event_n == g.InputEventsQueue.Size)
        g.InputEventsQueue.resize(0);
    else
        g.InputEventsQueue.erase(g.InputEventsQueue.Data, g.InputEventsQueue.Data + event_n);

    // Clear buttons state when focus is lost
    // - this is useful so e.g. releasing Alt after focus loss on Alt-Tab doesn't trigger the Alt menu toggle.
    // - we clear in EndFrame() and not now in order allow application/user code polling this flag
    //   (e.g. custom backend may want to clear additional data, custom widgets may want to react with a "canceling" event).
    if (g.IO.AppFocusLost)
    {
        g.IO.ClearInputKeys();
        g.IO.ClearInputMouse();
    }
}

ImGuiID ImGui::GetKeyOwner(ImGuiKey key)
{
    if (!IsNamedKeyOrMod(key))
        return ImGuiKeyOwner_NoOwner;

    ImGuiContext& g = *GImGui;
    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    ImGuiID owner_id = owner_data->OwnerCurr;

    if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != ImGuiKeyOwner_Any)
        if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
            return ImGuiKeyOwner_NoOwner;

    return owner_id;
}

// TestKeyOwner(..., ID)   : (owner == None || owner == ID)
// TestKeyOwner(..., None) : (owner == None)
// TestKeyOwner(..., Any)  : no owner test
// All paths are also testing for key not being locked, for the rare cases that key have been locked with using ImGuiInputFlags_LockXXX flags.
bool ImGui::TestKeyOwner(ImGuiKey key, ImGuiID owner_id)
{
    if (!IsNamedKeyOrMod(key))
        return true;

    ImGuiContext& g = *GImGui;
    if (g.ActiveIdUsingAllKeyboardKeys && owner_id != g.ActiveId && owner_id != ImGuiKeyOwner_Any)
        if (key >= ImGuiKey_Keyboard_BEGIN && key < ImGuiKey_Keyboard_END)
            return false;

    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    if (owner_id == ImGuiKeyOwner_Any)
        return owner_data->LockThisFrame == false;

    // Note: SetKeyOwner() sets OwnerCurr. It is not strictly required for most mouse routing overlap (because of ActiveId/HoveredId
    // are acting as filter before this has a chance to filter), but sane as soon as user tries to look into things.
    // Setting OwnerCurr in SetKeyOwner() is more consistent than testing OwnerNext here: would be inconsistent with getter and other functions.
    if (owner_data->OwnerCurr != owner_id)
    {
        if (owner_data->LockThisFrame)
            return false;
        if (owner_data->OwnerCurr != ImGuiKeyOwner_NoOwner)
            return false;
    }

    return true;
}

// _LockXXX flags are useful to lock keys away from code which is not input-owner aware.
// When using _LockXXX flags, you can use ImGuiKeyOwner_Any to lock keys from everyone.
// - SetKeyOwner(..., None)              : clears owner
// - SetKeyOwner(..., Any, !Lock)        : illegal (assert)
// - SetKeyOwner(..., Any or None, Lock) : set lock
void ImGui::SetKeyOwner(ImGuiKey key, ImGuiID owner_id, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(IsNamedKeyOrMod(key) && (owner_id != ImGuiKeyOwner_Any || (flags & (ImGuiInputFlags_LockThisFrame | ImGuiInputFlags_LockUntilRelease)))); // Can only use _Any with _LockXXX flags (to eat a key away without an ID to retrieve it)
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetKeyOwner) == 0); // Passing flags not supported by this function!
    //IMGUI_DEBUG_LOG("SetKeyOwner(%s, owner_id=0x%08X, flags=%08X)\n", GetKeyName(key), owner_id, flags);

    ImGuiKeyOwnerData* owner_data = GetKeyOwnerData(&g, key);
    owner_data->OwnerCurr = owner_data->OwnerNext = owner_id;

    // We cannot lock by default as it would likely break lots of legacy code.
    // In the case of using LockUntilRelease while key is not down we still lock during the frame (no key_data->Down test)
    owner_data->LockUntilRelease = (flags & ImGuiInputFlags_LockUntilRelease) != 0;
    owner_data->LockThisFrame = (flags & ImGuiInputFlags_LockThisFrame) != 0 || (owner_data->LockUntilRelease);
}

// Rarely used helper
void ImGui::SetKeyOwnersForKeyChord(ImGuiKeyChord key_chord, ImGuiID owner_id, ImGuiInputFlags flags)
{
    if (key_chord & ImGuiMod_Ctrl)      { SetKeyOwner(ImGuiMod_Ctrl, owner_id, flags); }
    if (key_chord & ImGuiMod_Shift)     { SetKeyOwner(ImGuiMod_Shift, owner_id, flags); }
    if (key_chord & ImGuiMod_Alt)       { SetKeyOwner(ImGuiMod_Alt, owner_id, flags); }
    if (key_chord & ImGuiMod_Super)     { SetKeyOwner(ImGuiMod_Super, owner_id, flags); }
    if (key_chord & ~ImGuiMod_Mask_)    { SetKeyOwner((ImGuiKey)(key_chord & ~ImGuiMod_Mask_), owner_id, flags); }
}

// This is more or less equivalent to:
//   if (IsItemHovered() || IsItemActive())
//       SetKeyOwner(key, GetItemID());
// Extensive uses of that (e.g. many calls for a single item) may want to manually perform the tests once and then call SetKeyOwner() multiple times.
// More advanced usage scenarios may want to call SetKeyOwner() manually based on different condition.
// Worth noting is that only one item can be hovered and only one item can be active, therefore this usage pattern doesn't need to bother with routing and priority.
void ImGui::SetItemKeyOwner(ImGuiKey key, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = g.LastItemData.ID;
    if (id == 0 || (g.HoveredId != id && g.ActiveId != id))
        return;
    if ((flags & ImGuiInputFlags_CondMask_) == 0)
        flags |= ImGuiInputFlags_CondDefault_;
    if ((g.HoveredId == id && (flags & ImGuiInputFlags_CondHovered)) || (g.ActiveId == id && (flags & ImGuiInputFlags_CondActive)))
    {
        IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetItemKeyOwner) == 0); // Passing flags not supported by this function!
        SetKeyOwner(key, id, flags & ~ImGuiInputFlags_CondMask_);
    }
}

void ImGui::SetItemKeyOwner(ImGuiKey key)
{
    SetItemKeyOwner(key, ImGuiInputFlags_None);
}

// This is the only public API until we expose owner_id versions of the API as replacements.
bool ImGui::IsKeyChordPressed(ImGuiKeyChord key_chord)
{
    return IsKeyChordPressed(key_chord, ImGuiInputFlags_None, ImGuiKeyOwner_Any);
}

// This is equivalent to comparing KeyMods + doing a IsKeyPressed()
bool ImGui::IsKeyChordPressed(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    key_chord = FixupKeyChord(key_chord);
    ImGuiKey mods = (ImGuiKey)(key_chord & ImGuiMod_Mask_);
    if (g.IO.KeyMods != mods)
        return false;

    // Special storage location for mods
    ImGuiKey key = (ImGuiKey)(key_chord & ~ImGuiMod_Mask_);
    if (key == ImGuiKey_None)
        key = ConvertSingleModFlagToKey(mods);
    if (!IsKeyPressed(key, (flags & ImGuiInputFlags_RepeatMask_), owner_id))
        return false;
    return true;
}

void ImGui::SetNextItemShortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags)
{
    ImGuiContext& g = *GImGui;
    g.NextItemData.HasFlags |= ImGuiNextItemDataFlags_HasShortcut;
    g.NextItemData.Shortcut = key_chord;
    g.NextItemData.ShortcutFlags = flags;
}

// Called from within ItemAdd: at this point we can read from NextItemData and write to LastItemData
void ImGui::ItemHandleShortcut(ImGuiID id)
{
    ImGuiContext& g = *GImGui;
    ImGuiInputFlags flags = g.NextItemData.ShortcutFlags;
    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedBySetNextItemShortcut) == 0); // Passing flags not supported by SetNextItemShortcut()!

    if (g.LastItemData.ItemFlags & ImGuiItemFlags_Disabled)
        return;
    if (flags & ImGuiInputFlags_Tooltip)
    {
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasShortcut;
        g.LastItemData.Shortcut = g.NextItemData.Shortcut;
    }
    if (!Shortcut(g.NextItemData.Shortcut, flags & ImGuiInputFlags_SupportedByShortcut, id) || g.NavActivateId != 0)
        return;

    // FIXME: Generalize Activation queue?
    g.NavActivateId = id; // Will effectively disable clipping.
    g.NavActivateFlags = ImGuiActivateFlags_PreferInput | ImGuiActivateFlags_FromShortcut;
    //if (g.ActiveId == 0 || g.ActiveId == id)
    g.NavActivateDownId = g.NavActivatePressedId = id;
    NavHighlightActivated(id);
}

bool ImGui::Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags)
{
    return Shortcut(key_chord, flags, ImGuiKeyOwner_Any);
}

bool ImGui::Shortcut(ImGuiKeyChord key_chord, ImGuiInputFlags flags, ImGuiID owner_id)
{
    ImGuiContext& g = *GImGui;
    //IMGUI_DEBUG_LOG("Shortcut(%s, flags=%X, owner_id=0x%08X)\n", GetKeyChordName(key_chord, g.TempBuffer.Data, g.TempBuffer.Size), flags, owner_id);

    // When using (owner_id == 0/Any): SetShortcutRouting() will use CurrentFocusScopeId and filter with this, so IsKeyPressed() is fine with he 0/Any.
    if ((flags & ImGuiInputFlags_RouteTypeMask_) == 0)
        flags |= ImGuiInputFlags_RouteFocused;

    // Using 'owner_id == ImGuiKeyOwner_Any/0': auto-assign an owner based on current focus scope (each window has its focus scope by default)
    // Effectively makes Shortcut() always input-owner aware.
    if (owner_id == ImGuiKeyOwner_Any || owner_id == ImGuiKeyOwner_NoOwner)
        owner_id = GetRoutingIdFromOwnerId(owner_id);

    if (g.CurrentItemFlags & ImGuiItemFlags_Disabled)
        return false;

    // Submit route
    if (!SetShortcutRouting(key_chord, flags, owner_id))
        return false;

    // Default repeat behavior for Shortcut()
    // So e.g. pressing Ctrl+W and releasing Ctrl while holding W will not trigger the W shortcut.
    if ((flags & ImGuiInputFlags_Repeat) != 0 && (flags & ImGuiInputFlags_RepeatUntilMask_) == 0)
        flags |= ImGuiInputFlags_RepeatUntilKeyModsChange;

    if (!IsKeyChordPressed(key_chord, flags, owner_id))
        return false;

    // Claim mods during the press
    SetKeyOwnersForKeyChord(key_chord & ImGuiMod_Mask_, owner_id);

    IM_ASSERT((flags & ~ImGuiInputFlags_SupportedByShortcut) == 0); // Passing flags not supported by this function!
    return true;
}

#endif // #ifndef IMGUI_DISABLE
