#pragma once

// Self-contained on purpose: see imgui_core_private.h for the rationale.
#include "imgui.h"

// Shared constant: 'inline constexpr' gives a single definition across all translation units
// instead of one internal-linkage copy per including TU (which 'static const' would produce).
inline constexpr ImU32 GDefaultRgbaColorMarkers[4] =
{
    IM_COL32(240,20,20,255), IM_COL32(20,240,20,255), IM_COL32(20,20,240,255), IM_COL32(140,140,140,255)
};
