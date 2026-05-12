#include <Component/Simple/LoggerUI.h>

#include <Theme.h> // 主题
#include <EditorUIWarehouse.h>

///////////////////////////////////////
#include <ILog.h>
///////////////////////
// 序列化文件相关
#include <AbsolutePath.h>
#include <FileManager.h>
#include <fstream>
#include <Serialize/SerializeMacro.h>
// 编码转换
#include <Converter.h>
#include <Component/EditorGeneralLayout.h>
///////////////////////
///////////////////////////////////////

namespace RenderUI {
    RENDERUI_REGISTER(LoggerUI);
    using namespace LOG;

    class SetBackColor {
    public:
        SetBackColor(ImVec4 color, ImGuiCol_ col = ImGuiCol_WindowBg) {
            ImGui::PushStyleColor(col, color);
        }
        ~SetBackColor() {
            ImGui::PopStyleColor();
        }
    };

    static ImVec4 GetLevelColor(LogLevel level) {
        switch (level) {
        case LogLevel::Debug:   return Theme::LOG_DEBUG;
        case LogLevel::Info:    return Theme::LOG_INFO;
        case LogLevel::Warning: return Theme::LOG_WARNING;
        case LogLevel::Error:   return Theme::LOG_ERROR;
        default:                return Theme::LOG_DEBUG;
        }
    }

    void LoggerUI::Init()
    {
        FILE_SERIALIZATION_LOADING(Switch, CONFIG "Editor\\Windows\\", L"LoggerSwitch.mtdata")
    }

    void LoggerUI::Uninstall()
    {
        FILE_SERIALIZATION_SAVE(Switch, CONFIG "Editor\\Windows\\", L"LoggerSwitch.mtdata")
    }

    void LoggerUI::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.LoggerWindow) {

            LoggerSwitch& LogSwitch = this->Switch;

            SetBackColor color(ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

            ImGui::Begin("Engine Log");

            // ---- 工具栏 ----
            if (ImGui::Button("Clear")) {
                GetLogInstance()->ClearRecentEntries();
            }
            ImGui::SameLine();

            // 级别过滤复选框
            ImGui::Checkbox("Debug", &LogSwitch.showDebug);
            ImGui::SameLine();
            ImGui::Checkbox("Info", &LogSwitch.showInfo);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &LogSwitch.showWarning);
            ImGui::SameLine();
            ImGui::Checkbox("Error", &LogSwitch.showError);

            ImGui::SameLine();
            ImGui::Checkbox("Auto-scroll", &LogSwitch.autoScroll);

            ImGui::Separator();

            // ---- 日志列表 ----
            ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            {
                const std::deque<LogEntry>& logs = GetLogInstance()->GetEntries();

                ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                for (const auto& entry : logs) {
                    if (entry.level == LogLevel::Debug && !LogSwitch.showDebug)   continue;
                    if (entry.level == LogLevel::Info && !LogSwitch.showInfo)    continue;
                    if (entry.level == LogLevel::Warning && !LogSwitch.showWarning) continue;
                    if (entry.level == LogLevel::Error && !LogSwitch.showError)   continue;

                    ImGui::PushID(&entry);
                    ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));

                    // 格式化的日志文本
                    std::string lineText = "[" + entry.timestamp + "] [" +
                        GetLogInstance()->GetLevelString(entry.level) + "] | " +
                        entry.message;

                    //ImGui::Text(lineText.c_str());
                    ImGui::Dummy(ImVec2(2.0, 0));
                    ImGui::SameLine(0.0, -1.0);
                    ImGui::Selectable(lineText.c_str(), false,
                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);

                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("File: %s", entry.file.c_str());
                        ImGui::Text("Line: %d", entry.line);
                        ImGui::EndTooltip();
                    }

                    // 复制
                    if (ImGui::BeginPopupContextItem("LogContext")) {
                        if (ImGui::MenuItem("Copy")) {
                            ImGui::SetClipboardText(lineText.c_str());
                        }
                        if (ImGui::MenuItem("Copy Source Path")) {
                            ImGui::SetClipboardText((entry.file + " " + std::to_string(entry.line)).c_str());
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }

                if (LogSwitch.autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();
            }

            // 自动滚动
            if (LogSwitch.autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
            ImGui::End();
        }
    }

    void* LoggerUI::PublicData(uint8_t type)
    {
        return nullptr;
    }
}