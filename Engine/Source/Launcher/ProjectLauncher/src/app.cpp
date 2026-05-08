#include "Main.h"
#include <FileManager.h>
#include <WebView2/WebView2.h>
#include <IProjectController.h>

#include <wrl/client.h> 
#include <wrl/event.h>
#include <string>
#include <unordered_set>

using namespace Microsoft::WRL;

HWND g_hwnd = nullptr;
std::atomic<bool> scan_ready(false);
size_t scan_old_info_size;
ComPtr<ICoreWebView2Controller> g_controller;
ComPtr<ICoreWebView2> g_webview;
CallbackFunc WebViewSuccess = SHOW_WINDOW;

void ProjectList::init()
{
    std::wstring exe_path =  IO::AbsolutePath::Get().GetExecutableDirectory();
    config_path = exe_path + L"\\" PTAH_JSON;

    if (IO::FileManager::Exists(config_path))
    {
        std::string Dump = IO::FileManager::ReadAllText(config_path);

        if (!Dump.empty()) {
            try
            {
                JSON items = JSON::parse(Dump);
                for (const auto& item : items) {
                    ProjectInfo Info;
                    Info.fromJson(item);
                    infos.push_back(Info);
                }
            }
            catch (const std::exception&)
            {
                OutputDebugStringA("JSON parsing failed!");
            }
            
        }
    }
}

void ProjectList::close()
{
    save();
}

void ProjectList::add(JSON J)
{
    ProjectInfo Info;
    Info.fromJson(J);
    infos.push_back(Info);

    save();
}

void ProjectList::save()
{
    JSON arr = JSON::array();
    for (auto& info : infos) {
        JSON j_info = info.toJson();
        arr.push_back(j_info);
    }
    std::string jsonStr = arr.dump();

    std::wstring config_dir;
    if (IO::FileManager::GetFileDirectory(config_path, config_dir)) {
        IO::FileManager::MakeDirectory(config_dir);
        IO::FileManager::MakeFile(config_path);
        IO::FileManager::WriteAllText(config_path, jsonStr);
    }
}

JSON ProjectList::GetInfos()
{
    JSON arr = JSON::array();
    for (auto& info : infos) {
        JSON j_info = info.toJson();
        arr.push_back(j_info);
    }
    return arr;
}

void ProjectList::scan()
{
    // ------------------------------------------- 去重
    std::vector<ProjectInfo> result;
    std::unordered_set<std::string> seen;
    for (auto& info : infos) {
        std::string key = info.name + "|" + info.path;  // 组合键
        if (seen.find(key) == seen.end()) {
            seen.insert(key);
            result.push_back(info);
        }
    }
    infos = std::move(result);

    // ------------------------------------------- 项目是否完整
    for (auto it = infos.begin(); it != infos.end(); ) {
        EngineProject::ProJectConfig config;
        config.name = it->name;
        config.path = it->path;

        if (!EngineProject::GetProjectControllerInterface()->ProjectComplete(config)) {
            it = infos.erase(it); 
        }
        else {
            ++it;  // 继续下一个
        }
    }

    save();
    scan_ready = true;
}

bool ProjectList::Existence(std::string path, std::string name)
{
    for (auto& info : infos)
    {
        if (info.name == name && info.path == path) {
            return true;
        }
    }
    return false;
}

void init_app(HWND hwnd) {
    g_hwnd = hwnd;
    std::wstring userDataFolder = IO::AbsolutePath::Get().GetExecutableDirectory() + L"\\" + CACHE + L"WebViewCache";

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userDataFolder.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result)) return S_OK;

                env->CreateCoreWebView2Controller(
                    g_hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result)) return S_OK;

                            g_controller = controller;
                            g_controller->get_CoreWebView2(&g_webview);

                            // 设置 WebView 大小
                            RECT bounds = { 0, 0, winW, winH };
                            g_controller->put_Bounds(bounds);

                            std::wstring src = IO::AbsolutePath::Get().GetContentPath() + L"\\ProjectLauncher\\index.html";
                            g_webview->Navigate(src.c_str());

#ifndef _DEBUG
                            // 禁用右键菜单和开发者工具
                            ComPtr<ICoreWebView2Settings> settings;
                            g_webview->get_Settings(&settings);
                            if (settings) {
                                settings->put_AreDefaultContextMenusEnabled(FALSE);
                                settings->put_AreDevToolsEnabled(FALSE);
                                settings->put_IsZoomControlEnabled(FALSE);
                            }
#endif

                            g_webview->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        //WebViewSuccess(); 我们使用web 中的就绪回调
                                        return S_OK;
                                    }).Get(),nullptr);

                            g_webview->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        LPWSTR rawMsg = nullptr;
                                        args->TryGetWebMessageAsString(&rawMsg);
                                        if (!rawMsg) return S_OK;

                                        std::wstring msg(rawMsg);
                                        CoTaskMemFree(rawMsg);

                                        JSON json = JSON::parse(msg);
                                        Operation(json);

                                        return S_OK;
                                    }).Get(),nullptr);

                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
}

void SHOW_WINDOW()
{
    ShowWindow(g_hwnd, SW_SHOWNORMAL);  // 激活并显示，恢复原有大小
    SetForegroundWindow(g_hwnd);
    UpdateWindow(g_hwnd);
}

void SendJSONToJS(const JSON& json) {
    if (g_webview) {
        std::string jsonStr = json.dump();
        std::wstring wJsonStr(jsonStr.begin(), jsonStr.end());
        g_webview->PostWebMessageAsJson(wJsonStr.c_str());
    }
}

void OpenProject(std::string path, std::string name) {
    std::wstring src = IO::AbsolutePath::Get().GetContentPath() + L"\\ProjectLauncher\\loading.html";
    g_webview->Navigate(src.c_str());
    // TODO
}

void Tick()
{
    // scan
    if (scan_ready)
    {
        scan_ready = false;
        // scan 完成
        if (scan_old_info_size != ProjectList::Get().GetSize()) {
            
            {
                JSON list = ProjectList::Get().GetInfos();
                JSON msg;
                msg["action"] = "project_list";
                msg["list"] = list;
                SendJSONToJS(msg);
            }
            {
                JSON msg;
                msg["action"] = "project_refresh";
                SendJSONToJS(msg);
            }
        }
    }
    // ....
}
