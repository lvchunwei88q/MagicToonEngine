#pragma once

#include <imgui.h>

namespace Theme {
    // ── Accent / brand color (Magic purple #7d449c) ─────────────────────
    constexpr float ACCENT_R = 0.490f;
    constexpr float ACCENT_G = 0.267f;
    constexpr float ACCENT_B = 0.612f;

    // ── Console log level colors ───────────────────────────────
    constexpr ImVec4 LOG_DEBUG{ 0.5f, 0.5f, 0.5f, 1.0f };// Debug
    constexpr ImVec4 LOG_INFO{ 1.0f, 1.0f, 1.0f, 1.0f };// Info
    constexpr ImVec4 LOG_WARNING{ 1.0f, 1.0f, 0.0f, 1.0f };// Warning
    constexpr ImVec4 LOG_ERROR{ 1.0f, 0.0f, 0.0f, 1.0f };// Error
}