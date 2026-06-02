#include "Editor/LoggerUI.h"

#include <Theme.h> // 主题
#include <RenderUIWarehouse.h>

// 序列化文件相关
#include <IO.h> 
#include <fstream>
#include <Tools/SerializeMacro.h>
// 序列化
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制

#include "Editor/EditorGeneralLayout.h"
#include <Tools/TextLayout.h>
///////////////////////
///////////////////////////////////////

namespace RenderEditor {
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

    auto CalcCheckboxWidth = [](const char* label) {
        float checkboxSquare = ImGui::GetFrameHeight();  // Checkbox 方框宽度 = 高度（正方形）
        float padding = ImGui::GetStyle().FramePadding.x * 2;
        float innerSpacing = checkboxSquare > 0 ? ImGui::GetStyle().ItemInnerSpacing.x : 0;
        float textWidth = ImGui::CalcTextSize(label).x;
        return checkboxSquare + innerSpacing + padding + textWidth;
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
        ViewSwitch& Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.LoggerWindow) {

            LoggerSwitch& LogSwitch = this->Switch;

            SetBackColor color(ImVec4(0.08f, 0.08f, 0.08f, 1.0f));

            ImGui::Begin("Engine Log",&Switch.LoggerWindow);

            // ---- 工具栏 ----
            {
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

                float usedWidth = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x;
                float AutoScrollCheckboxWidth = CalcCheckboxWidth("Auto-scroll");
                float DetailedInformationCheckboxWidth = CalcCheckboxWidth("Detailed-information");
                float WindowWidth = ImGui::GetWindowWidth();

                if (WindowWidth >= (AutoScrollCheckboxWidth + DetailedInformationCheckboxWidth + usedWidth)) {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - AutoScrollCheckboxWidth - DetailedInformationCheckboxWidth);
                    ImGui::Checkbox("Auto-scroll", &LogSwitch.autoScroll);
                    ImGui::SameLine();
                    ImGui::Checkbox("Detailed-information", &LogSwitch.detailedInformation);
                }
                else if (WindowWidth >= (DetailedInformationCheckboxWidth + usedWidth)) {
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - DetailedInformationCheckboxWidth);
                    ImGui::Checkbox("Detailed-information", &LogSwitch.detailedInformation);
                }
            }

            ImGui::Separator();

            // ---- 日志列表 ----
            float RemainingHeight = ImGui::GetContentRegionAvail().y;
            float Height = RemainingHeight - 200 > 0 && LogSwitch.detailedInformation ? RemainingHeight - 100 : RemainingHeight;
            ImGui::BeginChild("LogScrollRegion", ImVec2(0, Height), false, ImGuiWindowFlags_HorizontalScrollbar);

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

                    // 监听左键单击
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        Current_Log_Src = entry.file + ":" + std::to_string(entry.line);
                        Current_Log_Info = lineText;
                        Current_Log_Level = entry.level;
                    }

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

            if ((RemainingHeight - Height) > 0.0f && LogSwitch.detailedInformation) {
                SetBackColor color(ImVec4(0.10f, 0.10f, 0.10f, 1.0f), ImGuiCol_ChildBg);
                ImGui::BeginChild("LogContentRegion", ImVec2(0, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders,0);
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));
                    ImGui::BeginChild("LogContentInfoRegion", ImVec2(0, ImGui::GetContentRegionAvail().y - 20));
                    {
                        textlayout::TextLayoutParams params;
                        params.text = Current_Log_Info;
                        params.wrapWidth = ImGui::GetContentRegionAvail().x;
                        params.lineHeight = 1.2f;
                        params.letterSpacing = 0.0f;
                        textlayout::TextLayoutResult layout = textlayout::LayoutText(params);

                        if (layout.font == nullptr || layout.lines.empty()){}
                        else {
                            ImDrawList* drawList = ImGui::GetWindowDrawList();
                            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                            float boxHeight = layout.totalHeight;
                            ImVec2 min = ImVec2(cursorPos.x + 5, cursorPos.y + 5);
                            ImVec2 max = ImVec2(cursorPos.x + ImGui::GetContentRegionAvail().x - 5, cursorPos.y + boxHeight - 5);

                            drawList->PushTextureID(ImGui::GetIO().Fonts->TexRef);
                            textlayout::RenderTextBox(drawList, min.x, min.y, max.x, max.y,
                                layout, ImGui::ColorConvertFloat4ToU32(GetLevelColor(Current_Log_Level)), 0.0f, 0.0f, params.letterSpacing);
                            drawList->PopTextureID();

                            ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, boxHeight));
                        }
                    }
                    ImGui::EndChild();

                    ImGui::BeginChild("LogContentSrcRegion", ImVec2(0, ImGui::GetContentRegionAvail().y));
                    {
                        ImGui::Dummy(ImVec2(2.0, 0));
                        ImGui::SameLine(0.0, -1.0);
                        ImGui::Text("File: %s", Current_Log_Src.c_str());
                    }
                    ImGui::EndChild();

                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
            }

            ImGui::End();
        }
    }

    void* LoggerUI::PublicData(uint8_t type)
    {
        return nullptr;
    }
}