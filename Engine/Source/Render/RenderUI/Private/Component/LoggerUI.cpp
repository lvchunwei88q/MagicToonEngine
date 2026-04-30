#include <GuiInterface.h>

///////////////////////////////////////
#include <Log.h>
///////////////////////////////////////

namespace RenderUI {
    using namespace LOG;

    static ImVec4 GetLevelColor(LogLevel level) {
        switch (level) {
        case LogLevel::Debug:   return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // Gray
        case LogLevel::Info:    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White
        case LogLevel::Warning: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
        case LogLevel::Error:   return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
        default:                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    void LoggerWindow()
    {
        if (Switch.LoggerWindow) {

            LoggerSwitch& LogSwitch = Switch.loggerswitch;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
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
            ImGui::PopStyleColor();
        }
    }
}