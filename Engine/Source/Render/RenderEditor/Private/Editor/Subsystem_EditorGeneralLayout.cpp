#include <RenderUIWarehouse.h>

#include "Editor/EditorGeneralLayout.h"
///////////////////////
// 序列化文件相关
#include <IO.h> 
#include <fstream>
#include <SerializeMacro.h>

// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制
///////////////////////

namespace RenderEditor {
	RENDERUI_REGISTER(EditorGeneralLayout);

	void EditorGeneralLayout::Init()
	{
		FILE_SERIALIZATION_LOADING(Switch, CONFIG "Editor\\Windows\\", L"EditorSwitch.mtdata")
	}

	void EditorGeneralLayout::Uninstall()
	{
		FILE_SERIALIZATION_SAVE(Switch, CONFIG "Editor\\Windows\\", L"EditorSwitch.mtdata")
	}

	void* EditorGeneralLayout::PublicData(uint8_t type)
	{
		switch (static_cast<EditorGeneralLayoutData>(type))
		{
			case EditorGeneralLayoutData::ViewSwitch:{
				return static_cast<void*>(&Switch);
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