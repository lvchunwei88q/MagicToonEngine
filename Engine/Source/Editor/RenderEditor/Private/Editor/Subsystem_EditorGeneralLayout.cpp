#include <RenderUIWarehouse.h>

#include "Editor/EditorGeneralLayout.h"

namespace RenderEditor {
	RENDERUI_REGISTER(EditorGeneralLayout);

	void EditorGeneralLayout::Init()
	{
		Switch = ObjectFactory::CreateUnique<ViewSwitch>();
	}

	void EditorGeneralLayout::Uninstall()
	{
		Switch.reset();
	}

	void* EditorGeneralLayout::PublicData(uint8_t type)
	{
		switch (static_cast<EditorGeneralLayoutData>(type))
		{
			case EditorGeneralLayoutData::ViewSwitch:{
				return static_cast<void*>(Switch.get());
			}
		}

		return nullptr;
	}

	void EditorGeneralLayout::ExitProgram()
	{
		RenderUIContext UIContext = GetSubsystem()->GetRenderUIContext();
		HWND hwnd = reinterpret_cast<HWND>(UIContext.hwnd);
		PostMessage(hwnd, WM_CLOSE, 0, 0); // Close
	}
}