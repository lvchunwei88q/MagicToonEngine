#include "Main.h"
#include <shobjidl.h>
#include <Converter.h>

#include <IProjectController.h>
#include <Core.h>
#include <thread>

// 全局变量
bool g_isDragging = false;
POINT g_lastMousePos;
HHOOK g_mouseHook = nullptr;

// MouseProc
void StopDrag();
void StartDrag(HWND hwnd);

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && g_isDragging) {
		if (wParam == WM_LBUTTONUP) {  // 左键松开
			StopDrag();
		}
        if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;

            int dx = pMouseStruct->pt.x - g_lastMousePos.x;
            int dy = pMouseStruct->pt.y - g_lastMousePos.y;

            HWND hwnd = GetForegroundWindow(); // 或者保存窗口句柄
            RECT rect;
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, nullptr,
                rect.left + dx, rect.top + dy,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            g_lastMousePos = pMouseStruct->pt;
        }
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

void StartDrag(HWND hwnd) {
	g_isDragging = true;
	GetCursorPos(&g_lastMousePos);
	SetCapture(hwnd);
	g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc,
		GetModuleHandle(nullptr), 0);
}

void StopDrag() {
	g_isDragging = false;
	ReleaseCapture();

	if (g_mouseHook) {
		UnhookWindowsHookEx(g_mouseHook);
		g_mouseHook = nullptr;
	}
}

std::wstring BrowseFolder(HWND hwnd) {
	IFileDialog* pfd = nullptr;
	std::wstring result;

	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC,
		IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr)) {
		DWORD dwOptions;
		pfd->GetOptions(&dwOptions);
		pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);

		// 显示对话框
		hr = pfd->Show(hwnd);
		if (SUCCEEDED(hr)) {
			IShellItem* psi;
			pfd->GetResult(&psi);

			PWSTR pszPath;
			psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
			result = pszPath;
			CoTaskMemFree(pszPath);
			psi->Release();
		}
		pfd->Release();
	}
	return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CREATE: // 窗口创建
	{
		init_app(hwnd);
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

void Operation(JSON json)
{
	std::string action = json["action"];
	if (action == "create_projnew_projectct") {
		std::string name = json["name"];
		std::string path = json["path"];
		NewProJect(path,name);
	}
	else if(action == "web_ready") {
		WebViewSuccess(); // show
		{
			JSON j = ProjectList::Get().GetInfos();
			JSON msg;
			msg["action"] = "project_list";
			msg["list"] = j;
			SendJSONToJS(msg);
		}
		{
			JSON msg;
			msg["action"] = "engine_version";
			msg["version"] = Core::Core::GetVersion();
			SendJSONToJS(msg);
		}
	}
	else if (action == "start_drag") {
		StartDrag(g_hwnd);
	}else if(action == "web_close") {
		PostQuitMessage(0);
	}
	else if (action == "web_scan_project") {
		scan_ready = false;
		scan_old_info_size = ProjectList::Get().GetSize();
		std::thread t(&ProjectList::scan, &ProjectList::Get());
		t.detach();
	}
	else if (action == "web_browse") {
		JSON msg;
		msg["action"] = "browse_path";
		msg["path"] = IO::Converter::ToNarrowString(BrowseFolder(g_hwnd));
		SendJSONToJS(msg);
	}
	else if (action == "web_open_project") {
		std::string name = json["name"];
		std::string path = json["path"];
		OpenProject(path, name);
	}
}

using namespace EngineProject;

void NewProJect(std::string path, std::string name)
{
	if (!ProjectList::Get().Existence(path, name)) {
		IProjectController* controller = GetProjectControllerInterface();

		ProJectConfig config;
		config.name = name;
		config.path = path;
		controller->Create(config);

		JSON j;
		j["name"] = name;
		j["path"] = path;
		j["version"] = Core::Core::GetVersion();
		ProjectList::Get().add(j);

		{
			JSON list = ProjectList::Get().GetInfos();
			JSON msg;
			msg["action"] = "project_list";
			msg["list"] = list;
			SendJSONToJS(msg);
		}

		{
			JSON msg;
			msg["action"] = "project_create_success";
			SendJSONToJS(msg);
		}
	}
	else {
		JSON msg;
		msg["action"] = "project_existence";
		SendJSONToJS(msg);
	}
}