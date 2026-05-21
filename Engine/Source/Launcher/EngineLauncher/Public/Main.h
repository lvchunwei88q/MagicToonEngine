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

namespace EngineLauncher
{
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

    // windows event
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void Tick();
}