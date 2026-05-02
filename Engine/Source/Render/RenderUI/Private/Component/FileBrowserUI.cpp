#include <GuiInterface.h>

///////////////////////////////////////
#include <Log.h>

#include <AbsolutePath.h>
#include <Converter.h>
#include <FileManager.h>
///////////////////////////////////////

#include <filesystem>
#include <functional>
#include <vector>
#include <string>

namespace RenderUI {
    namespace fs = std::filesystem;

    // 内容浏览器状态
    struct ContentBrowserState {
        std::wstring rootPath;                          // 根目录
        std::wstring currentPath;                       // 当前浏览的目录（宽字符）
        std::string selectedFilePath;                   // 当前选中的文件（窄字符，相对于当前目录）
        std::function<void(const std::string&)> onFileDoubleClicked; // 双击文件回调
    };

    // 全局实例
    static ContentBrowserState g_contentBrowser;

    void InitContentBrowser();
    void DrawDirectoryTree();
    void DrawContentBrowser();

    void DrawContentBrowserWindow()
    {
        static bool init = false;
        if (!init) {
            init = true;
            InitContentBrowser();
        }

        DrawContentBrowser();
    }

    void InitContentBrowser() {
        g_contentBrowser.currentPath = IO::AbsolutePath::Get().GetCurrentWorkingDirectory();
        g_contentBrowser.rootPath = IO::AbsolutePath::Get().GetCurrentWorkingDirectory();
    }

    void DrawDirectoryTree(const std::wstring& rootPath) {
        fs::path root(rootPath);
        for (const auto& entry : fs::directory_iterator(root)) {
            if (!entry.is_directory()) continue;

            std::string dirName = IO::Converter::ToNarrowString(entry.path().filename().wstring());
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool isCurrent = (entry.path() == g_contentBrowser.currentPath);
            if (isCurrent)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool opened = ImGui::TreeNodeEx(dirName.c_str(), flags);
            if (ImGui::IsItemClicked()) {
                g_contentBrowser.currentPath = entry.path().wstring();
            }
            if (opened) {
                DrawDirectoryTree(entry.path().wstring());
                ImGui::TreePop();
            }
        }
    }

    void DrawContentBrowser() {
        SetBackColor color(ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

        ImGui::Begin("Content Browser");

        // 顶部路径栏
        {
            std::string curPath = IO::Converter::ToNarrowString(g_contentBrowser.currentPath);
            ImGui::Text("Path: %s", curPath.c_str());
            ImGui::SameLine();
            if(ImGui::Button("Up")) {
                fs::path p(g_contentBrowser.currentPath);
                fs::path root(g_contentBrowser.rootPath);
                // 只有当前路径不是根目录时才向上
                if (p != root) {
                    g_contentBrowser.currentPath = p.parent_path().wstring();
                }
            }
        }
        ImGui::Separator();

        static float dirTreeWidth = 250.0f;
        static float splitterWidth = 4.0f;

        // 左右分割
        // 左侧：目录树
        ImGui::BeginChild("DirTree", ImVec2(dirTreeWidth, 0), true);
        {
            static std::wstring rootPath = IO::AbsolutePath::Get().GetExecutableDirectory();
            DrawDirectoryTree(rootPath);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // 分割条
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Button("##splitter", ImVec2(splitterWidth, -1));
        ImGui::PopStyleColor(3);

        if (ImGui::IsItemActive()) {
            dirTreeWidth += ImGui::GetIO().MouseDelta.x;
            if (dirTreeWidth < 100.0f) dirTreeWidth = 100.0f;  // 最小宽度
            if (dirTreeWidth > 500.0f) dirTreeWidth = 500.0f;  // 最大宽度
        }

        // 鼠标悬停时改变光标
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SameLine();

        // 右侧：当前目录内容（文件夹 + 文件）- 网格布局
        ImGui::BeginChild("Files", ImVec2(0, 0), true);
        {
            fs::path curDir(g_contentBrowser.currentPath);
            if (!fs::exists(curDir)) {
                ImGui::Text("Directory does not exist.");
                ImGui::EndChild();
                ImGui::End();
                return;
            }

            // 计算网格参数
            float itemSize = 80.0f;        // 每个格子的大小
            float spacing = 10.0f;         // 格子间距
            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columns = (int)(panelWidth / (itemSize + spacing));
            if (columns < 1) columns = 1;

            int itemIndex = 0;

            // 文件夹列表
            for (const auto& entry : fs::directory_iterator(curDir)) {
                if (!entry.is_directory()) continue;

                std::string name = IO::Converter::ToNarrowString(entry.path().filename().wstring());

                // 新行
                if (itemIndex % columns != 0) ImGui::SameLine(0, spacing);

                ImGui::BeginGroup();
                ImGui::PushID(itemIndex);

                // 文件夹图标/按钮区域
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.7f, 0.2f, 0.6f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.8f, 0.3f, 0.8f));

                if (ImGui::Button("[DIR]", ImVec2(itemSize, itemSize))) {
                    static double lastClickTime = 0.0;
                    double now = ImGui::GetTime();

                    if (now - lastClickTime < 0.4) {  // 0.4秒内双击
                        g_contentBrowser.currentPath = entry.path().wstring();
                    }
                    lastClickTime = now;
                }
                ImGui::PopStyleColor(2);

                // 文件名（截断过长文本）
                std::string displayName = name;
                if (displayName.length() > 8)
                    displayName = displayName.substr(0, 7) + "...";
                ImGui::TextWrapped("%s", displayName.c_str());

                ImGui::EndGroup();
                ImGui::PopID();

                itemIndex++;
            }

            // 文件列表
            for (const auto& entry : fs::directory_iterator(curDir)) {
                if (!entry.is_regular_file()) continue;

                std::string name = IO::Converter::ToNarrowString(entry.path().filename().wstring());
                bool isSelected = (entry.path().string() == g_contentBrowser.selectedFilePath);

                // 新行
                if (itemIndex % columns != 0) ImGui::SameLine(0, spacing);

                ImGui::BeginGroup();
                ImGui::PushID(itemIndex);

                // 根据文件类型设置颜色
                std::string ext = entry.path().extension().string();
                ImVec4 btnColor = ImVec4(0.5f, 0.5f, 0.5f, 0.6f);
                if (isSelected)
                    btnColor = ImVec4(0.3f, 0.6f, 1.0f, 0.8f);

                ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x + 0.1f, btnColor.y + 0.1f, btnColor.z + 0.1f, 0.8f));

                // 文件类型标签
                std::string fileLabel = "[" + ext.substr(1) + "]";
                std::transform(fileLabel.begin(), fileLabel.end(), fileLabel.begin(), ::toupper);

                if (ImGui::Button(fileLabel.c_str(), ImVec2(itemSize, itemSize))) {
                    g_contentBrowser.selectedFilePath = entry.path().string();
                    if (ImGui::IsMouseDoubleClicked(0) && g_contentBrowser.onFileDoubleClicked) {
                        g_contentBrowser.onFileDoubleClicked(g_contentBrowser.selectedFilePath);
                    }
                }
                ImGui::PopStyleColor(2);

                // 文件名
                std::string displayName = name;
                if (displayName.length() > 8)
                    displayName = displayName.substr(0, 7) + "...";
                ImGui::TextWrapped("%s", displayName.c_str());

                ImGui::EndGroup();
                ImGui::PopID();

                itemIndex++;
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }
}