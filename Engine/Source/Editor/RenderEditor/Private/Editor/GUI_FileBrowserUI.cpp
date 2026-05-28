#include "Editor/FileBrowserUI.h"
#include <IO.h> 
#include <Tools/TextLayout.h>

#include "Editor/EditorGeneralLayout.h"

namespace RenderEditor {

    class SetBackColor {
    public:
        SetBackColor(ImVec4 color, ImGuiCol_ col = ImGuiCol_WindowBg) {
            ImGui::PushStyleColor(col, color);
        }
        ~SetBackColor() {
            ImGui::PopStyleColor();
        }
    };

    auto HighlightedArea = [](ImVec2 pos, ImVec2 size) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddRectFilled(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(100, 100, 100, 60),
            4.0f
        );
    };

    void FileBrowserUI::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.ContentBrowserWindow) {
            // render
            DrawLayout();
            //DirTree
            //File
        }
    }

    void FileBrowserUI::DrawLayout()
    {
        SetBackColor color(ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

        ImGui::Begin("Content Browser");

        // path
        {
            std::string curPath = IO::ToNarrowString(state.currentPath);

            // 输入框缓冲区
            static char pathBuffer[512] = {};
            strcpy_s(pathBuffer, curPath.c_str());

            if (ImGui::InputText("##PathEdit", pathBuffer, sizeof(pathBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue))
            {
                std::wstring newPath = IO::ToWideString(pathBuffer);
                fs::path p(newPath);

                // 安全检查
                if (IsUnderGameFolder(p) && IO::Exists(GetAbsolutePath(newPath))) {
                    state.currentPath = newPath;
                }
                else {
                    LOG_WARNING("Invalid path, restoring original path!");
                    strcpy_s(pathBuffer, curPath.c_str());
                }
            }

            ImGui::SameLine();
            {
                ImVec2 cursorPos = ImGui::GetCursorScreenPos(); // Get Render Postion

                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::InvisibleButton("##Return-Navigation-bar", ImVec2(25, 25));

                bool isHovered = ImGui::IsItemHovered();
                ID3D11ShaderResourceView* Icon = GetFileIcon(UIAssetType::Return);

                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::Image(
                    (ImTextureID)(uintptr_t)Icon,
                    ImVec2(25, 25)
                );

                if (isHovered)
                {
                    HighlightedArea(cursorPos, { 25,25 });

                    if (ImGui::IsMouseClicked(0)) {
                        // return
                        // 如果已经是 "Game\\"，不再往上
                        if (state.currentPath == L"Game\\")
                        {
                            LOG_INFO("The path to reach the top");
                        }
                        else {
                            fs::path p(state.currentPath);
                            fs::path parent = p.parent_path();
                            // 如果是只有Game（Root）那么加入一个\\否则不加
                            state.currentPath = parent.wstring() == L"Game" ? parent.wstring() + L"\\" : parent.wstring();
                        }
                    }
                }
            }
        }
        ImGui::Separator();

        // main content
        {
            {
                ImGui::BeginChild("DirTree", ImVec2(Config.dirTreeWidth, 0), true);
                {
                    DirTreePos = ImGui::GetCursorPos();
                    this->DirTree();
                }
                ImGui::EndChild();
            }

            ImGui::SameLine(0.0f, 0.0f);
            {
                ImVec2 splitterSize(splitterWidth, ImGui::GetContentRegionAvail().y);

                ImGui::InvisibleButton("##splitter", splitterSize);

                if (ImGui::IsItemActive()) {
                    Config.dirTreeWidth += ImGui::GetIO().MouseDelta.x;
                    Config.dirTreeWidth = ImClamp(Config.dirTreeWidth, 100.0f, ImGui::GetContentRegionAvail().x * 0.6f);
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                }

                ImDrawList* draw = ImGui::GetWindowDrawList();
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                bool active = ImGui::IsItemActive();
                ImU32 color = active ? IM_COL32(170, 170, 170, 255)
                    : ImGui::IsItemHovered() ? IM_COL32(100, 100, 100, 255)
                    : IM_COL32(50, 50, 50, 255);
                draw->AddRectFilled(min, max, color);
            }

            ImGui::SameLine(0.0f, 0.0f);
            {
                // file list
                SetBackColor color(ImVec4(0.10f, 0.10f, 0.10f, 1.0f), ImGuiCol_ChildBg);
                ImGui::BeginChild("Files", ImVec2(0, 0), true);
                {
                    FilePos = ImGui::GetCursorPos();
                    FileContentAreaInput();
                    RightClickMenuBar(); // 右键菜单
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

            std::string dirName = IO::ToNarrowString(entry.path().filename().wstring());
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
        if (!IO::Exists(GetAbsolutePath(state.currentPath))) {
            ImGui::Text("Directory does not exist.");
            return;
        }

        auto TextCentered = [](const char* text, float containerWidth,
            float fontSize = 0.0f,
            const std::string& fontPath = "",
            float lineHeight = 1.0f,
            float letterSpacing = 0.0f)
            {
                textlayout::TextLayoutParams params;
                params.text = text;
                params.fontPath = fontPath;
                params.fontSize = fontSize;
                params.wrapWidth = containerWidth;  // 自动换行
                params.lineHeight = lineHeight;
                params.letterSpacing = letterSpacing;

                textlayout::TextLayoutResult layout = textlayout::LayoutText(params);

                if (layout.font == nullptr || layout.lines.empty())
                    return;

                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 pos = ImGui::GetCursorScreenPos();

                // 创建绘制区域
                ImVec2 min = pos;
                ImVec2 max = { pos.x + containerWidth, pos.y + layout.totalHeight };

                drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
                textlayout::RenderTextBox(drawList, min.x, min.y, max.x, max.y,
                    layout, IM_COL32(255, 255, 255, 255),
                    0.5f, 0.0f, letterSpacing);  // alignX=0.5 水平居中
                drawList->PopTextureID();

                // 预留空间
                ImGui::Dummy(ImVec2(containerWidth, layout.totalHeight));
            };

        // 计算网格参数
        float panelWidth = ImGui::GetContentRegionAvail().x;
        const float Zoom = Config.Zoom; // 获取缩放
        const float ZitemSize = itemSize * Zoom;
        const float Zspacing = spacing * Zoom;
        int columns = (int)(panelWidth / (ZitemSize + Zspacing));
        if (columns < 1) columns = 1;

        int itemIndex = 0;


        // File List
        for (const auto& entry : fs::directory_iterator(GetAbsolutePath(state.currentPath))) {

            std::string name = IO::ToNarrowString(entry.path().filename().wstring());

            UIAssetType FileType = GetFileType(entry);

            FileTypeContext context;
            context.entry_path = entry.path();
            context.IsFolder = FileType == UIAssetType::Folder;

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
                ID3D11ShaderResourceView* Icon = GetFileIcon(FileType);

                ImGui::SetCursorScreenPos(cursorPos);
                ImGui::Image(
                    (ImTextureID)(uintptr_t)Icon,
                    ImVec2(ZitemSize, ZitemSize)
                );
                if (isHovered) {
                    HighlightedArea(cursorPos, { ZitemSize,ZitemSize });
                }

                RightClickMenuBar_File(context); // bind 右键菜单

                if (isHovered)
                {
                    DoubleClickToEnter(context);
                }

                // FileName
                std::string displayName = name;
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

    void FileBrowserUI::DoubleClickToEnter(FileTypeContext Context)
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            if (Context.IsFolder)
                state.currentPath = ToGameRelative(Context.entry_path.wstring());
            else {
                if (Context.entry_path.extension() != "") { // TODO file type 
                    FileOperations(Context);
                }
                else {
                    LOG_WARNING("Unrecognized file type!");
                }
            }
        }
    }

    void FileBrowserUI::RightClickMenuBar()
    {
        if (ImGui::BeginPopupContextWindow("FileBrowser_RightClickMenu",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverExistingPopup))
        {
            // 空白区域菜单
            if (ImGui::MenuItem("Reset Size")) {
                LOG_INFO("Reset Size for file browser...");
                Config.Zoom = 1.0f;
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Folder")) { NewFolder(); }
                // TODO more type
                // if (ImGui::MenuItem("Material")) { ... }
                // if (ImGui::MenuItem("Script")) { ... }
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::EndPopup();
        }
    }

    void FileBrowserUI::RightClickMenuBar_File(FileTypeContext Context)
    {
        if (ImGui::BeginPopupContextItem("FileItemMenu"))
        {
            if (ImGui::MenuItem("Open")) {
                if (Context.IsFolder) { // open
                    state.currentPath = ToGameRelative(Context.entry_path.wstring());
                }
                else {
                    FileOperations(Context);
                }
            }
            if (ImGui::MenuItem("Delete")) {
                if (Context.IsFolder) {
                    DeleteFolderOrFile(Context.entry_path.filename().wstring());
                }
                else {
                    DeleteFolderOrFile(Context.entry_path.filename().wstring(), true);
                }
            }
            ImGui::EndPopup();
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
}