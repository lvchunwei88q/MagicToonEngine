#include "Editor/EditorSettings.h"
#include "Editor/EditorGeneralLayout.h"
#include <RenderUIWarehouse.h>
#include <imgui_internal.h>

namespace RenderEditor {
    static int selectedModule = 0;

    void EditorSettings::Tick()
    {
        ViewSwitch Switch = *(ViewSwitch*)GetSubsystem()->GetSubsystemPublicData("EditorGeneralLayout", (uint8_t)EditorGeneralLayoutData::ViewSwitch);
        if (Switch.EditorSettingsView) {
            // Set style
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7.0f, 7.0f));

            // set windows flags
            ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse;

            ImGui::Begin(Lang::Get("editor.setting.title").c_str(), nullptr, windowFlags);
            {
                // Model
                ImGui::BeginChild("SettingsModules", ImVec2(ModulesWidth, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
                {
                    std::string modules[] = { Lang::Get("editor.setting.general"), Lang::Get("editor.setting.language") };
                    
                    for (int i = 0; i < IM_ARRAYSIZE(modules); i++) {
                        if (ImGui::Selectable(modules[i].c_str(), selectedModule == i)) {
                            selectedModule = i;
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine(0.0f, 0.0f);
                {
                    ImVec2 splitterSize(splitterWidth, ImGui::GetContentRegionAvail().y);

                    ImGui::InvisibleButton("##splitter", splitterSize);

                    if (ImGui::IsItemActive()) {
                        ModulesWidth += ImGui::GetIO().MouseDelta.x;
                        ModulesWidth = ImClamp(ModulesWidth, 50.0f, ImGui::GetContentRegionAvail().x * 0.6f);
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

                ImGui::BeginChild("SettingsContent", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
                {
                    switch (selectedModule) {
                    case 0: // General
                    {
                        ImGui::Text(Lang::Get("editor.setting.general").c_str());
                        ImGui::Separator();
                    }
                        break;
                    case 1: // Language
                    {
                        ImGui::Text(Lang::Get("editor.setting.language").c_str());
                        ImGui::Separator();
                        {
                            int language = static_cast<int>(Setting.language);
                            std::string english = Lang::Get("editor.setting.english");
                            std::string chinese = Lang::Get("editor.setting.chinese");

                            const char* languages[] = { english.c_str(),chinese.c_str() };
                            if (ImGui::Combo(Lang::Get("editor.setting.language").c_str(), &language, languages, IM_ARRAYSIZE(languages))) {
                                OnUserSelectLanguage(static_cast<Languages>(language));
                                LoadLanguageFile(nullptr);
                            }
                        }
                    }
                        break;
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();

            ImGui::PopStyleVar();
        }
    }
}