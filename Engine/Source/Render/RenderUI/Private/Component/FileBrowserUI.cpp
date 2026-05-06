#include <Component/FileBrowserUI.h>
///////////////////////////////////////
#include <ILog.h>

#include <AbsolutePath.h>
#include <Converter.h>
#include <FileManager.h>

#include <Tools/Debouncer.h>
///////////////////////////////////////

#include <functional>
#include <vector>
#include <string>

namespace RenderUI {
    AUTO_REGISTER(FileBrowserUI)

    bool FileBrowserUI::Init()
    {
        state.currentPath = literal_root; // game ==  rootPath
        state.rootPath = IO::AbsolutePath::Get().GetCurrentWorkingDirectory(); // 设置根路径就是工作路径

        return true;
    }

    void FileBrowserUI::Uninstall()
    {
    }

    void FileBrowserUI::Draw()
    {
        // render
        DrawLayout();
        //DirTree
        //File
    }

    void FileBrowserUI::DrawLayout()
    {
        SetBackColor color(ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

        ImGui::Begin("Content Browser");

        // path
        {
            std::string curPath = IO::Converter::ToNarrowString(state.currentPath);

            // 输入框缓冲区
            static char pathBuffer[512] = {};
            strcpy_s(pathBuffer, curPath.c_str());

            if (ImGui::InputText("##PathEdit", pathBuffer, sizeof(pathBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                std::wstring newPath = IO::Converter::ToWideString(pathBuffer);
                fs::path p(newPath);

                // 安全检查
                if (IsUnderGameFolder(p) && IO::FileManager::Exists(GetAbsolutePath(newPath))) {
                    state.currentPath = newPath;
                }
                else {
                    LOG_WARNING("Invalid path, restoring original path!");
                    strcpy_s(pathBuffer, curPath.c_str());
                }
            }
        }
        ImGui::Separator();

        // main content
        {
            {
                ImGui::BeginChild("DirTree", ImVec2(dirTreeWidth, 0), true);
                {
                    DirTreePos = ImGui::GetCursorPos();
                    this->DirTree();
                }
                ImGui::EndChild();
            }

            ImGui::SameLine();
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::Button("##splitter", ImVec2(splitterWidth, -1));
                ImGui::PopStyleColor(3);

                if (ImGui::IsItemActive()) {
                    ImVec2 windowSize = ImGui::GetContentRegionAvail();

                    dirTreeWidth += ImGui::GetIO().MouseDelta.x;
                    if (dirTreeWidth < 100.0f) dirTreeWidth = 100.0f;  // 最小宽度
                    if (dirTreeWidth > windowSize.x * 0.6f) dirTreeWidth = windowSize.x * 0.6f;  // 最大宽度
                }

                // 鼠标悬停时改变光标
                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                }
            }

            ImGui::SameLine();
            {
                // file list
                SetBackColor color(ImVec4(0.14f, 0.14f, 0.14f, 1.0f), ImGuiCol_ChildBg);
                ImGui::BeginChild("Files", ImVec2(0, 0), true);
                {
                    FilePos = ImGui::GetCursorPos();
                    FileContentAreaInput();
                    this->File();
                }
                ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    void FileBrowserUI::DirTree()
    {
        fs::path root(state.rootPath);

        ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
        bool isCurrent = (literal_root == state.currentPath); // 当前是不是在Root中
        if (isCurrent)
            rootFlags |= ImGuiTreeNodeFlags_Selected;

        bool opened = ImGui::TreeNodeEx("Game", rootFlags); // 是否展开
        if (ImGui::IsItemClicked()) {
            state.currentPath = literal_root;
        }

        if (opened)
        {
            DrawDirectoryTree(state.rootPath);  // 原来的函数画子目录
            ImGui::TreePop();
        }
    }

    void FileBrowserUI::DrawDirectoryTree(std::wstring path) {
        fs::path root(path);
        for (const auto& entry : fs::directory_iterator(root)) {
            if (!entry.is_directory()) continue;

            std::string dirName = IO::Converter::ToNarrowString(entry.path().filename().wstring());
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool isCurrent = (entry.path() == GetAbsolutePath(state.currentPath));
            if (isCurrent)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool opened = ImGui::TreeNodeEx(dirName.c_str(), flags);
            if (ImGui::IsItemClicked()) {
                state.currentPath = ToGameRelative(entry.path().wstring());
            }
            if (opened) {
                DrawDirectoryTree(entry.path().wstring());
                ImGui::TreePop();
            }
        }
    }

    void FileBrowserUI::File()
    {
        if (!IO::FileManager::Exists(GetAbsolutePath(state.currentPath))) {
            ImGui::Text("Directory does not exist.");
            return;
        }

        auto TextCentered = [](const char* text, float containerWidth)
            {
                float textWidth = ImGui::CalcTextSize(text).x;
                float offsetX = (containerWidth - textWidth) * 0.5f;
                if (offsetX > 0)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                ImGui::TextUnformatted(text);
            };

        // 计算网格参数
        float panelWidth = ImGui::GetContentRegionAvail().x;
        const float Zoom = Switch.filebrowserconfig.Zoom; // 获取缩放
        const float ZitemSize = itemSize * Zoom;
        const float Zspacing = spacing * Zoom;
        int columns = (int)(panelWidth / (ZitemSize + Zspacing));
        if (columns < 1) columns = 1;

        int itemIndex = 0;


        // File List
        for (const auto& entry : fs::directory_iterator(GetAbsolutePath(state.currentPath))) {

            std::string name = IO::Converter::ToNarrowString(entry.path().filename().wstring());

            EngineAssetType FileType = GetFileType(entry);

            // New Line
            if (itemIndex % columns != 0) ImGui::SameLine(0, Zspacing);

            ImGui::BeginGroup();
            ImGui::PushID(itemIndex);

            // File UI
            {
                ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Get Render Postion

                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##IconClick", ImVec2(ZitemSize, ZitemSize));

                bool isHovered = ImGui::IsItemHovered();
                ID3D11ShaderResourceView* Icon = GetFileIcon(FileType,isHovered);

                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::Image(
                    (ImTextureID)(uintptr_t)Icon,
                    ImVec2(ZitemSize, ZitemSize)
                );

                if (isHovered)
                {
                    DoubleClickToEnter({ FileType == EngineAssetType::Folder, entry.path()});
                }

                // FileName
                std::string displayName = name;
                if (displayName.length() > 8)
                    displayName = displayName.substr(0, 7) + "...";
                TextCentered(displayName.c_str(), ZitemSize);
            }

            ImGui::PopID();
            ImGui::EndGroup();

            itemIndex++;
        }
    }

    bool FileBrowserUI::IsUnderGameFolder(const fs::path& path) {
        std::wstring native = path.lexically_normal().wstring();
        // 必须以 "Game\" 或 "Game/" 开头才算
        return native.starts_with(L"Game\\") || native.starts_with(L"Game/");
    }

    void FileBrowserUI::DoubleClickToEnter(FileTypeContext filetype)
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            if(filetype.IsFolder)
                state.currentPath = ToGameRelative(filetype.entry.wstring());
            else {
                if (filetype.entry.extension() != "") { // TODO file type 
                    // not
                }
                else {
                    LOG_WARNING("Unrecognized file type!");
                }
            }
        }
    }

    std::wstring FileBrowserUI::GetAbsolutePath(const std::wstring& currentPath)
    {
        std::wstring relative = currentPath;
        if (relative.starts_with(L"Game/") || relative.starts_with(L"Game\\")) {
            relative = relative.substr(5);  // 跳过 "Game/"
        }
        if (!relative.empty() && (relative[0] == L'/' || relative[0] == L'\\')) {
            relative = relative.substr(1);
        }
        fs::path absolute = fs::path(state.rootPath) / relative;
        return absolute.wstring();
    }

    std::wstring FileBrowserUI::ToGameRelative(const std::wstring& absolutePath)
    {
        fs::path root(state.rootPath);
        fs::path absolute(absolutePath);
        fs::path relative = fs::relative(absolute, root); // 计算相对路径

        // 前面拼上 "Game/"
        fs::path result = fs::path(L"Game") / relative;
        return result.lexically_normal().wstring();
    }

    EngineAssetType FileBrowserUI::GetFileType(const fs::directory_entry& entry)
    {
        if (entry.is_directory())
            return EngineAssetType::Folder;

        std::string ext = entry.path().extension().string();
        // TODO file type

        return EngineAssetType::File;
    }

    ID3D11ShaderResourceView* FileBrowserUI::GetFileIcon(EngineAssetType filetype,bool isHovered)
    {
        return RenderCore::GetBufferManagerUserInterface()->GetRTextureSRV(GetEngineAsset()->GetIcon(filetype, isHovered));
    }

    void FileBrowserUI::FileContentAreaInput()
    {
        ImGuiIO& io = ImGui::GetIO();
        bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

        static Debouncer debouncer(300, []() {
            LOG_INFO("FileBrowser Zoom changed to: " + std::to_string(Switch.filebrowserconfig.Zoom));
        });

        if (isHovered) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && io.MouseWheel != 0.0f) {
                float wheelDelta = io.MouseWheel;
                float& Zoom = Switch.filebrowserconfig.Zoom; // 获取缩放
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

    //------------------------------------------------------
    void DrawContentBrowserWindow()
    {
        if (Switch.ContentBrowserWindow) {
            FileBrowserUI::Get().Draw();
        }
    }
}