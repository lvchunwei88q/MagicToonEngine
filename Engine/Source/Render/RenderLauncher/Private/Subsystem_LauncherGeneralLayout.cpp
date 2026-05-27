#include "LauncherGeneralLayout.h"
#include <RenderUI.h>

///////////////////////
#include <IO.h> 
///////////////////////

namespace RenderLauncher {
	RENDERUI_REGISTER(LauncherGeneralLayout);

	const char* testJson = R"([
                {"name":"GameTEST","path":"D:\\Test\\MTE","version":"0.0.1.8"},
                {"name":"NewGameTEST","path":"D:\\Test\\MTE","version":"0.2.0.2"},
                {"name":"AAAA","path":"D:\\Test\\MTE","version":"0.2.0.4"}
            ])";

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

		std::string JsonDump = IO::ReadAllText(ContentPath);
		UIContext.EditorLanguage = JSON::parse(JsonDump);
		UIContext.EditorLanguageHash = ComputeStringHash(JsonDump);

		// 加载 UI 相关资源
		new_btn  = new MenuButton( Lang::Get("launcher.menu.new")  );
		open_btn = new MenuButton( Lang::Get("launcher.menu.open") );
		exit_btn = new MenuButton( Lang::Get("launcher.menu.exit") );

		this->LoadProjectList();
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

	bool LauncherGeneralLayout::LoadProjectList() {
		try {
			auto jsonData = nlohmann::json::parse(testJson);
			for (const auto& item : jsonData) {
				ProjectInfo info;
				info.name = item.value("name", "");
				info.path = item.value("path", "");
				info.version = item.value("version", "");
				if (!info.name.empty()) {
					projects.push_back(info);
				}
			}

			return true;
		}
		catch (const std::exception& e) {
			// 解析失败时打印错误
			std::string errorMsg = std::string("Failed to parse test JSON: ") + e.what();
			IM_ASSERT(false && errorMsg.c_str());

			return false;
		}
	}

	void LauncherGeneralLayout::OpenProject(std::string path, std::string name) {
		SetUIType(UIType::Loading); // 设置为加载界面

		// 构建 MagicEditor.exe 路径
		std::wstring exeDir = IO::AbsolutePath::Get().GetExecutableDirectory();
		std::wstring exePath = exeDir + L"\\MagicEditor.exe";

		// 构建命令行参数：-p "path" "name"
		std::wstring cmdLine = L"\"" + exePath + L"\"";
		cmdLine += L" -p \"";
		cmdLine += IO::ToWideString(path);
		cmdLine += L"\" \"";
		cmdLine += IO::ToWideString(name);
		cmdLine += L"\"";

		// 启动进程
		STARTUPINFOW si = { sizeof(si) };
		PROCESS_INFORMATION pi = {};
		BOOL success = CreateProcessW(
			nullptr,
			&cmdLine[0],
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&si,
			&pi
		);
	}

	void LauncherGeneralLayout::ExitProgram()
	{
		RenderUIContext UIContext = GetSubsystem()->GetRenderUIContext();
		HWND hwnd = reinterpret_cast<HWND>(UIContext.hwnd);
		PostMessage(hwnd, WM_CLOSE, 0, 0); // Close
	}
}