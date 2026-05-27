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

		std::string JsonDump = IO::FileManager::ReadAllText(ContentPath);
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

	void LauncherGeneralLayout::ExitProgram()
	{
		RenderUIContext UIContext = GetSubsystem()->GetRenderUIContext();
		HWND hwnd = reinterpret_cast<HWND>(UIContext.hwnd);
		PostMessage(hwnd, WM_CLOSE, 0, 0); // Close
	}
}