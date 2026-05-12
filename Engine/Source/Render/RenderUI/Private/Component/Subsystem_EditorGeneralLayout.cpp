#include <Component/EditorGeneralLayout.h>

#include <EditorUIWarehouse.h>
///////////////////////
// 序列化文件相关
#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>
// 编码转换
#include <Converter.h>
///////////////////////

namespace RenderUI {
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
}