#pragma once

#include <windows.h>
#include <string>
#include <Tools/Singleton.h> // from Core
#include <AbsolutePath.h>
#include <atomic>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#define winW 960
#define winH 540

#define PTAH_JSON CONFIG L"Project\\project.list.json"

struct ProjectInfo {
	std::string path;
	std::string name;
	std::string version;

    // 转换为 JSON
    JSON toJson() {
        return {
            {"name", name},
            {"path", path},
            {"version", version},
        };
    }

    // 从 JSON 转换
    void fromJson(const JSON& j) {
        name = j.at("name").get<std::string>();
        path = j.at("path").get<std::string>();
        version = j.value("version", "0.0.0.0"); 
    }
};

class ProjectList : public Singleton<ProjectList> // from Core
{
public:
    void init();
    void close();

    void add(JSON J);
    void save();
    JSON GetInfos();
    void scan();

    bool Existence(std::string path, std::string name);

    size_t GetSize() { return infos.size(); }
private:
	std::vector<ProjectInfo> infos;
    std::wstring config_path;
};

// windows event
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

using CallbackFunc = void(*)();

void init_app(HWND hwnd);
void SHOW_WINDOW();
void SendJSONToJS(const JSON& json);

void Tick();

extern CallbackFunc WebViewSuccess;
extern HWND g_hwnd;
extern std::atomic<bool> scan_ready;
extern size_t scan_old_info_size;

// tools function
void Operation(JSON json);
void NewProJect(std::string path, std::string name);