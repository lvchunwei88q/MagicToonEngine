#include "LauncherGeneralLayout.h"
#include <RenderUI.h>

///////////////////////
#include <AbsolutePath.h>
#include <FileManager.h>
// 编码转换
#include <Converter.h>
///////////////////////

namespace RenderLauncher {
	RENDERUI_REGISTER(LauncherGeneralLayout);

	auto GetLanguageFileName = []() {
			std::wstring ContentPath = IO::AbsolutePath::Get().GetContentPath();
			ContentPath += L"\\Config\\Languages\\";
			ContentPath += L"launcher.json"; // 默认 English

			return ContentPath;
		};

	void LauncherGeneralLayout::Init()
	{
		RenderUIContext& UIContext = GetSubsystem()->SetRenderUIContext();
		std::wstring ContentPath = GetLanguageFileName();

		std::string JsonDump = IO::FileManager::ReadAllText(ContentPath);
		UIContext.EditorLanguage = JSON::parse(JsonDump);
		UIContext.EditorLanguageHash = ComputeStringHash(JsonDump);

		// 加载 UI 相关资源
		new_btn  = new MenuButton( Lang::Get("launcher.menu.new")  );
		open_btn = new MenuButton( Lang::Get("launcher.menu.open") );
		exit_btn = new MenuButton( Lang::Get("launcher.menu.exit") );
	}

	void LauncherGeneralLayout::Uninstall()
	{
	}

	void* LauncherGeneralLayout::PublicData(uint8_t type)
	{
		return nullptr;
	}

	size_t LauncherGeneralLayout::ComputeJSONHash(const JSON& data) {
		std::string dump = data.dump();  // 序列化为字符串
		return std::hash<std::string>{}(dump);
	}

	size_t LauncherGeneralLayout::ComputeStringHash(const std::string& data)
	{
		return std::hash<std::string>{}(data);
	}

	void LauncherGeneralLayout::ExitProgram()
	{
		RenderUIContext UIContext = GetSubsystem()->GetRenderUIContext();
		HWND hwnd = reinterpret_cast<HWND>(UIContext.hwnd);
		PostMessage(hwnd, WM_CLOSE, 0, 0); // Close
	}
}