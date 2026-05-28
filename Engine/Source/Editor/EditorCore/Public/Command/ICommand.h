// ICommand.h
#pragma once
#include <string>

namespace Command {
    class ICommand {
    public:
        virtual ~ICommand() = default;

        // 执行命令（第一次执行或重做时调用）
        virtual void Execute() = 0;

        // 撤销命令
        virtual void Undo() = 0;

        // 获取命令描述
        virtual std::string GetDescription() const = 0;

        // 判断是否可以与另一个命令合并
        virtual bool CanMergeWith(const ICommand* other) const { return false; }

        // 合并命令
        virtual void MergeWith(const ICommand* other) {}
    };
}