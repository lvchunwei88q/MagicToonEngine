#include "Tools/ImGuiNotify.h"

namespace RenderUI {
    void ImGuiToastManager::ClearNotifications()
    {
        notifications.clear();
    }

    void ImGuiToastManager::SetNotifications(const std::vector<ImGuiToast>& notifications)
    {
        this->notifications = notifications;
    }

    void ImGuiToastManager::AddNotification(const ImGuiToast& toast)
    {
        notifications.push_back(toast);
    }

    void ImGuiToastManager::RemoveNotification(size_t index)
    {
        if (index < notifications.size())
        {
            notifications.erase(notifications.begin() + index);
        }
    }

    std::vector<ImGuiToast>& ImGuiToastManager::GetNotifications()
    {
        return notifications;
    }

    const std::vector<ImGuiToast>& ImGuiToastManager::GetNotifications() const
    {
        return notifications;
    }

    size_t ImGuiToastManager::GetNotificationCount() const
    {
        return notifications.size();
    }

    namespace Notify {
        _Use_decl_annotations_
        void InsertNotification(const ImGuiToast& toast)
        {
            IGTM::Get().AddNotification(toast);
        }

        _Use_decl_annotations_
        void RemoveNotification(size_t index)
        {
            IGTM::Get().RemoveNotification(index);
        }

        _Use_decl_annotations_
        void RenderNotifications()
        {
            const ImVec2 mainWindowSize = ImGui::GetMainViewport()->Size;

            float height = 0.f;

            std::vector<ImGuiToast>& notifications = IGTM::Get().GetNotifications();

            for (size_t i = 0; i < notifications.size(); ++i)
            {
                ImGuiToast* currentToast = &notifications[i];

                // Remove toast if expired
                if (currentToast->getPhase() == ImGuiToastPhase::Expired)
                {
                    RemoveNotification(i);
                    continue;
                }

#if NOTIFY_RENDER_LIMIT > 0
                if (i > NOTIFY_RENDER_LIMIT)
                {
                    continue;
                }
#endif

                // Get icon, title and other data
                const char* icon = currentToast->getIcon();
                const char* title = currentToast->getTitle();
                const char* content = currentToast->getContent();
                const char* defaultTitle = currentToast->getDefaultTitle();
                const float opacity = currentToast->getFadePercent(); // Get opacity based of the current phase

                // Window rendering
                ImVec4 textColor = currentToast->getColor();
                textColor.w = opacity;

                // Generate new unique name for this toast
                char windowName[50];
#ifdef _WIN32
                sprintf_s(windowName, "##TOAST%d", (int)i);
#elif defined(__linux__) || defined(__EMSCRIPTEN__)
                std::sprintf(windowName, "##TOAST%d", (int)i);
#elif defined (__APPLE__)
                std::snprintf(windowName, 50, "##TOAST%d", (int)i);
#else
                throw "Unsupported platform";
#endif

                //PushStyleColor(ImGuiCol_Text, textColor);
                ImGui::SetNextWindowBgAlpha(opacity);

#if NOTIFY_RENDER_OUTSIDE_MAIN_WINDOW
                short mainMonitorId = static_cast<ImGuiViewportP*>(ImGui::GetMainViewport())->PlatformMonitor;

                ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
                ImGuiPlatformMonitor& monitor = platformIO.Monitors[mainMonitorId];

                // Set notification window position to bottom right corner of the monitor
                ImGui::SetNextWindowPos(ImVec2(monitor.WorkPos.x + monitor.WorkSize.x - NOTIFY_PADDING_X, monitor.WorkPos.y + monitor.WorkSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
#else
                // Set notification window position to bottom right corner of the main window, considering the main window size and location in relation to the display
                ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;
                ImGui::SetNextWindowPos(ImVec2(mainWindowPos.x + mainWindowSize.x - NOTIFY_PADDING_X, mainWindowPos.y + mainWindowSize.y - NOTIFY_PADDING_Y - height), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
#endif

                // Set notification window flags
                if (!NOTIFY_USE_DISMISS_BUTTON && currentToast->getOnButtonPress() == nullptr)
                {
                    currentToast->setWindowFlags(NOTIFY_DEFAULT_TOAST_FLAGS | ImGuiWindowFlags_NoInputs);
                }

                ImGui::Begin(windowName, nullptr, currentToast->getWindowFlags());

                // Render over all other windows
                ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

                // Here we render the toast content
                {
                    ImGui::PushTextWrapPos(mainWindowSize.x / 3.f); // We want to support multi-line text, this will wrap the text after 1/3 of the screen width

                    bool wasTitleRendered = false;

                    // If an icon is set
                    if (!NOTIFY_NULL_OR_EMPTY(icon))
                    {
                        //Text(icon); // Render icon text
                        ImGui::TextColored(textColor, "%s", icon);
                        wasTitleRendered = true;
                    }

                    // If a title is set
                    if (!NOTIFY_NULL_OR_EMPTY(title))
                    {
                        // If a title and an icon is set, we want to render on same line
                        if (!NOTIFY_NULL_OR_EMPTY(icon))
                            ImGui::SameLine();

                        ImGui::Text("%s", title); // Render title text
                        wasTitleRendered = true;
                    }
                    else
                        if (!NOTIFY_NULL_OR_EMPTY(defaultTitle))
                        {
                            if (!NOTIFY_NULL_OR_EMPTY(icon))
                                ImGui::SameLine();

                            ImGui::Text("%s", defaultTitle); // Render default title text (ImGuiToastType_Success -> "Success", etc...)
                            wasTitleRendered = true;
                        }

                    // If a dismiss button is enabled
                    if (NOTIFY_USE_DISMISS_BUTTON)
                    {
                        // If a title or content is set, we want to render the button on the same line
                        if (wasTitleRendered || !NOTIFY_NULL_OR_EMPTY(content))
                        {
                            ImGui::SameLine();
                        }

                        // Render the dismiss button on the top right corner
                        // NEEDS TO BE REWORKED
                        float scale = 0.8f;

                        if (ImGui::CalcTextSize(content).x > ImGui::GetWindowContentRegionMax().x)
                        {
                            scale = 0.95f;
                        }

                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetWindowSize().x - ImGui::GetCursorPosX()) * scale);

                        // If the button is pressed, we want to remove the notification
                        if (ImGui::Button(ICON_FA_XMARK))
                        {
                            RemoveNotification(i);
                        }
                    }

                    // In case ANYTHING was rendered in the top, we want to add a small padding so the text (or icon) looks centered vertically
                    if (wasTitleRendered && !NOTIFY_NULL_OR_EMPTY(content))
                    {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f); // Must be a better way to do this!!!!
                    }

                    // If a content is set
                    if (!NOTIFY_NULL_OR_EMPTY(content))
                    {
                        if (wasTitleRendered)
                        {
#if NOTIFY_USE_SEPARATOR
                            ImGui::Separator();
#endif
                        }

                        ImGui::Text("%s", content); // Render content text
                    }

                    // If a button is set
                    if (currentToast->getOnButtonPress() != nullptr)
                    {
                        // If the button is pressed, we want to execute the lambda function
                        if (ImGui::Button(currentToast->getButtonLabel()))
                        {
                            currentToast->getOnButtonPress()();
                        }
                    }

                    ImGui::PopTextWrapPos();
                }

                // Save height for next toasts
                height += ImGui::GetWindowHeight() + NOTIFY_PADDING_MESSAGE_Y;

                // End
                ImGui::End();
            }
        }
    }
}