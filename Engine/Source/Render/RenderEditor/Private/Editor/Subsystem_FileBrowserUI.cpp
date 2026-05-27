#include "Editor/FileBrowserUI.h"

#include <RenderUIWarehouse.h>
///////////////////////////////////////
#include <ILog.h>

#include <IO.h> 
#include <fstream>
#include <SerializeMacro.h>

#include <Tools/Debouncer.h>
///////////////////////////////////////

// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制

#include <functional>
#include <string>
#include "Tools/OpenToolsView.h"

namespace RenderEditor {
    RENDERUI_REGISTER(FileBrowserUI);

    void FileBrowserUI::Init()
    {
        state.currentPath = literal_root; // game ==  rootPath
        state.rootPath = IO::AbsolutePath::Get().GetCurrentWorkingDirectory(); // 设置根路径就是工作路径

        FILE_SERIALIZATION_LOADING(Config, CONFIG "Editor\\Windows\\", L"FileBrowserConfig.mtdata")
    }

    void FileBrowserUI::Uninstall()
    {
        FILE_SERIALIZATION_SAVE(Config, CONFIG "Editor\\Windows\\", L"FileBrowserConfig.mtdata")
    }

    void* FileBrowserUI::PublicData(uint8_t type)
    {
        return nullptr;
    }

    // --------------------------------------------------- Operation ------------------------------------------------------- //

    EngineAssetType FileBrowserUI::GetFileType(const fs::directory_entry& entry)
    {
        if (entry.is_directory())
            return EngineAssetType::Folder;

        std::string ext = entry.path().extension().string();
        // TODO file type
        if (ext == ".json") {
            return EngineAssetType::JSON;
        }

        return EngineAssetType::File;
    }

    ID3D11ShaderResourceView* FileBrowserUI::GetFileIcon(EngineAssetType filetype)
    {
        return RenderCore::GetBufferManagerUserInterface()->GetRTextureSRV(GetEngineAsset()->GetIcon(filetype));
    }

    void FileBrowserUI::FileContentAreaInput()
    {
        ImGuiIO& io = ImGui::GetIO();
        bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        static Debouncer debouncer(300, [this]() {
            LOG_INFO("FileBrowser Zoom changed to: ", Config.Zoom);
            });

        if (isHovered) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && io.MouseWheel != 0.0f) {
                float wheelDelta = io.MouseWheel;
                float& Zoom = Config.Zoom; // 获取缩放
                if (wheelDelta != 0.0f) {
                    float newZoom = Zoom + (wheelDelta / 50.0f);
                    newZoom = std::clamp(newZoom, 0.5f, 1.5f);
                    if (newZoom != Zoom) {
                        Zoom = newZoom;
                        debouncer.update();
                    }
                    io.MouseWheel = 0.0f;
                }
            }
        }

        debouncer.tick();
    }

    void FileBrowserUI::NewFolder()
    {
        OpenToolsView::Get().OpenToolsWindow(OpenToolsWindows::OpenTools::NewFolder);
    }

    void FileBrowserUI::DeleteFolderOrFile(std::wstring Terget, bool isfile)
    {
        std::wstring path = GetAbsolutePath(state.currentPath);
        path += L"\\" + Terget;

        if (isfile) {
            LOG_INFO("Delete File:" + IO::ToNarrowString(Terget));
            IO::DeleteToFile(path);
        }
        else {
            LOG_INFO("Delete Folder:" + IO::ToNarrowString(Terget));
            IO::DeleteToDirectory(path);
        }
    }

    void FileBrowserUI::FileOperations(FileTypeContext Context)
    {
        // TODO File Operation
    }
}