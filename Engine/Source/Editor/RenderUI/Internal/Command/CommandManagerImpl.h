// CommandManagerImpl.h
#pragma once
#include "Command/ICommand.h"
#include "Command/ICommandManager.h"
#include <Subsystem/SubsystemTemplate.h>

#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace Command {
    class CommandManagerImpl final : public SubsystemTemplate<CommandManagerImpl,Core::Priority::High> , public ICommandManager {
    private:
		std::vector<std::unique_ptr<ICommand>> m_undoStack;     // 撤销栈
		std::vector<std::unique_ptr<ICommand>> m_redoStack;     // 重做栈
        size_t m_maxStackSize = 50;  // 最大历史步数
        // 观察者
        std::vector<RegisterFunc> m_onStackChangedCallbacks;

    public:
        virtual bool Init() override;
        virtual void Uninstall() override;
        virtual void Notification(Core::NotificationContext Context) override;

        // 执行命令并记录到撤销栈
        void ExecuteCommand(std::unique_ptr<ICommand> command) override;
        // 注册UI更新回调
        void RegisterCallback(RegisterFunc callback) override { m_onStackChangedCallbacks.push_back(callback); }
        // 撤销
        void Undo() override;
        // 重做
        void Redo() override;
        // 清空所有历史
        void Clear() override;

        // 状态查询
        bool CanUndo() const override { return !m_undoStack.empty(); }
        bool CanRedo() const override { return !m_redoStack.empty(); }
        std::string GetUndoDescription() const override;
        std::string GetRedoDescription() const override;
    private:
        void NotifyStackChanged();
    };
}