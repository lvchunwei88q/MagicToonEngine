#pragma once

#include "Common/EDITORCORE_API.h"
#include "Command/ICommand.h"
#include <memory>
#include <functional>

namespace Command {
    using RegisterFunc = std::function<void()>;

	class EDITORCORE_API ICommandManager
	{
	public:
		virtual ~ICommandManager() = default;

        // 执行命令并记录到撤销栈
        virtual void ExecuteCommand(std::unique_ptr<ICommand> command) = 0;
        // 注册UI更新回调
        virtual void RegisterCallback(RegisterFunc callback) = 0;
        // 撤销
        virtual void Undo() = 0;
        // 重做
        virtual void Redo() = 0;
        // 清空所有历史
        virtual void Clear() = 0;
        // 状态查询
        virtual bool CanUndo() const = 0;
        virtual bool CanRedo() const = 0;
        virtual std::string GetUndoDescription() const = 0;
        virtual std::string GetRedoDescription() const = 0;
	};

    EDITORCORE_API ICommandManager* GetCommandManager();
}