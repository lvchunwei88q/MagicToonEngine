#include "Command/CommandManagerImpl.h"
#include <ILog.h>

namespace Command {
	AUTO_REGISTER_NOTIFICATION(CommandManagerImpl,"COMMAND");

	ICommandManager* GetCommandManager()
	{
		return &CommandManagerImpl::Get();
	}

	bool CommandManagerImpl::Init()
	{
		return true;
	}

	void CommandManagerImpl::Uninstall()
	{
	}

	void CommandManagerImpl::Notification(Core::NotificationContext Context)
	{
		switch (Context.tags)
		{
		case encodeToSizeT("Undo"): {
			Undo();
		}break;
		case encodeToSizeT("Redo"): {
			Redo();
		}break;
		default: LOG_ERROR("Unknown type");
			break;
		}
	}

	void CommandManagerImpl::ExecuteCommand(std::unique_ptr<ICommand> command)
	{
		// 执行命令
		command->Execute();

		// 检查是否可以与上一个命令合并
		if (!m_undoStack.empty() &&
			m_undoStack.back()->CanMergeWith(command.get())) {
			m_undoStack.back()->MergeWith(command.get());
		}
		else {
			// 添加到撤销栈
			m_undoStack.push_back(std::move(command));
			// 新命令会清空重做栈
			m_redoStack.clear();
		}

		// 限制栈大小
		while (m_undoStack.size() > m_maxStackSize) {
			m_undoStack.erase(m_undoStack.begin());
		}

		NotifyStackChanged();
	}

	void CommandManagerImpl::Undo()
	{
		if (m_undoStack.empty()) return;

		auto& command = m_undoStack.back();
		command->Undo();

		// 移动到重做栈
		m_redoStack.push_back(std::move(command));
		m_undoStack.pop_back();

		LOG_INFO("Undo: ", m_redoStack.back()->GetDescription());
		NotifyStackChanged();
	}

	void CommandManagerImpl::Redo()
	{
		if (m_redoStack.empty()) return;

		auto& command = m_redoStack.back();
		command->Execute();

		// 移回撤销栈
		m_undoStack.push_back(std::move(command));
		m_redoStack.pop_back();

		LOG_INFO("Redo: ", m_undoStack.back()->GetDescription());
		NotifyStackChanged();
	}

	void CommandManagerImpl::Clear()
	{
		m_undoStack.clear();
		m_redoStack.clear();
		NotifyStackChanged();
	}

	std::string CommandManagerImpl::GetUndoDescription() const
	{
		if (m_undoStack.empty()) return "";
		return "Undo " + m_undoStack.back()->GetDescription();
	}

	std::string CommandManagerImpl::GetRedoDescription() const
	{
		if (m_redoStack.empty()) return "";
		return "Redo " + m_redoStack.back()->GetDescription();
	}

	void CommandManagerImpl::NotifyStackChanged()
	{
		for (auto& callback : m_onStackChangedCallbacks) {
			callback();
		}
	}
}