#pragma once
#include <nlohmann/json.hpp>

namespace RenderUI {
	using JSON = nlohmann::json;

	// As a globally shared resource when rendering the UI
	struct RenderUIContext {
		uintptr_t hwnd;
		JSON EditorLanguage;
		size_t EditorLanguageHash = -1;
	};
}