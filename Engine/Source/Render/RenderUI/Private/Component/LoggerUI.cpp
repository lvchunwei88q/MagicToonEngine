#include <GuiInterface.h>

///////////////////////////////////////
#include <Log.h>
///////////////////////////////////////

namespace RenderUI {
    using namespace LOG;

    static bool showDebug;
    static bool showInfo;
    static bool showWarning;
    static bool showError;

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
            ImGui::Begin("Engine Log");

            // ---- 工具栏 ----
            if (ImGui::Button("Clear")) {
                //ClearRecentEntries();
            }
            ImGui::SameLine();

            // 级别过滤复选框
            ImGui::Checkbox("Debug", &showDebug);
            ImGui::SameLine();
            ImGui::Checkbox("Info", &showInfo);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &showWarning);
            ImGui::SameLine();
            ImGui::Checkbox("Error", &showError);

            ImGui::SameLine();
            static bool autoScroll = true;
            ImGui::Checkbox("Auto-scroll", &autoScroll);

            ImGui::Separator();

            // ---- 日志列表 ----
            ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            {
                const std::deque<LogEntry>& logs = GetLogInstance()->GetEntries();

                ImGuiListClipper clipper;
                clipper.Begin(static_cast<int>(logs.size()));

                while (clipper.Step()) {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                        const auto& entry = logs[i];

                        // 根据级别过滤
                        if (entry.level == LogLevel::Debug && !showDebug)   continue;
                        if (entry.level == LogLevel::Info && !showInfo)    continue;
                        if (entry.level == LogLevel::Warning && !showWarning) continue;
                        if (entry.level == LogLevel::Error && !showError)   continue;

                        // 着色
                        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));

                        // 格式：[时间] [级别] 文件:行号 | 消息
                        ImGui::Text("[%s] [%s] %s:%d | %s",
                            entry.timestamp.c_str(),
                            "Info",
                            entry.file.c_str(),
                            entry.line,
                            entry.message.c_str());

                        ImGui::PopStyleColor();
                    }
                }
                clipper.End();
            }

            // 自动滚动
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
            ImGui::End();
        }
    }
}