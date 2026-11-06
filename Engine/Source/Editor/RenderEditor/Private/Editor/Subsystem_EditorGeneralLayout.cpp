#include <RenderUIWarehouse.h>
#include <IO.h>

#include "Editor/EditorGeneralLayout.h"

namespace RenderEditor {
	RENDERUI_REGISTER(EditorGeneralLayout);

	void EditorGeneralLayout::Init()
	{
		Switch = ObjectFactory::CreateUnique<ViewSwitch>();
		const std::wstring configDir = IO::AbsolutePath::Get().GetExecutableDirectory() + L"\\Config\\Editor\\";
		const std::wstring configPath = configDir + L"ViewSwitch.mtdata";
		if (IO::Exists(configPath)) {
			std::vector<uint8_t> configFileData = IO::ReadAllUINT8Bytes(configPath);
			BinaryWriter configData;
			configData.Write(configFileData.data(), configFileData.size());
			Switch->Deserialize(configData);
		}
	}

	void EditorGeneralLayout::Uninstall()
	{
		const std::wstring configDir = IO::AbsolutePath::Get().GetExecutableDirectory() + L"\\Config\\Editor\\";
		IO::CreateDirectory(configDir);
		const BinaryReader configData = Switch->Serialize();
		IO::WriteAllBytes(configDir + L"ViewSwitch.mtdata", configData.DataU8(), configData.Size());
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