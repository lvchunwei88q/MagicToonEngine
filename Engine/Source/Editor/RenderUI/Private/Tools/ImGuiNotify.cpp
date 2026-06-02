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
}