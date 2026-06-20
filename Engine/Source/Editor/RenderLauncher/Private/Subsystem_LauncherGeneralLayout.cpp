#include "LauncherGeneralLayout.h"
#include <RenderUI.h>
#include <Window/WindowAppointment.hpp>

///////////////////////
#include <IO.h> 
///////////////////////

namespace RenderLauncher {
	RENDERUI_REGISTER(LauncherGeneralLayout);
	AUTO_REGISTER_NOTIFICATION(LauncherData, "LAUNCHERDATA");

	const char* testJson = R"([
                {"name":"GameTEST","path":"D:\\Test\\MTE","version":"0.0.1.8"},
                {"name":"NewGameTEST","path":"D:\\Test\\MTE","version":"0.2.0.2"},
                {"name":"AAAA","path":"D:\\Test\\MTE","version":"0.2.0.4"}
            ])";

	auto GetLanguageFileName = []() {
			std::wstring ContentPath = IO::AbsolutePath::Get().GetContentDirectory().GetConfig();
			ContentPath += L"\\Languages\\";
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
		new_btn  = new MenuButton(std::string(ICON_FA_FILE)        , Lang::Get("launcher.menu.new") );
		open_btn = new MenuButton(std::string(ICON_FA_FOLDER_OPEN) , Lang::Get("launcher.menu.open"));
		exit_btn = new MenuButton(std::string(ICON_FA_DOOR_OPEN)   , Lang::Get("launcher.menu.exit"));

		this->LoadProjectList();
	}

	void LauncherGeneralLayout::Uninstall()
	{
	}

	void* LauncherGeneralLayout::PublicData(uint8_t type)
	{
		return nullptr;
	}

	bool LauncherData::Init()
	{
		return true;
	}

	void LauncherData::Uninstall()
	{
	}

	void LauncherData::Notification(Core::NotificationContext Context)
	{
		switch (Context.tags)
		{
		case encodeToSizeT("LOADVALUE"): {
			const char* pData = Context.msg;
			// 先读 int：字符串长度
			int nameLen = *(int*)pData;
			pData += sizeof(int);
			// 再读 float：进度
			float percent = *(float*)pData;
			pData += sizeof(float);
			// 最后读字符串
			std::string name((char*)pData, nameLen);

			LauncherDataContext::Get().current_subsystem = name;
			LauncherDataContext::Get().percent = percent;

			if (percent >= 0.99f) {
				// End
				//Sleep(300);
				//PostQuitMessage(0);
			}
		}break;
		default:LOG_ERROR("Unknown type");
			break;
		}
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
		Core::GetSubsystemContext()->Notification("Application", { encodeToSizeT("SETWINDOW"),nullptr});

		// 构建 ENGINE_EDITOR_NAME_W 路径
		std::wstring exeDir = IO::AbsolutePath::Get().GetExecutableDirectory();
		std::wstring exePath = exeDir + L"\\" ENGINE_EDITOR_NAME_W ".exe";

		// 构建命令行参数：--project_path "path" --name "name"
		std::wstring cmdLine = L"\"" + exePath + L"\"";
		cmdLine += L" --project_path \"";
		cmdLine += IO::ToWideString(path);
		cmdLine += L"\" --name \"";
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