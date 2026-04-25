#pragma once
#include <Common/IO_API.h>
#include <Common/Singleton.h>
#include <string>

#define CACHE L"Cache\\"

namespace IO {
	class IO_API AbsolutePath : public Singleton<AbsolutePath> // 绝对路径子系统
	{
	public:
		AbsolutePath() = default;

		/**
		 * 获取可执行文件所在目录
		 * @return 可执行文件的完整目录路径（不包含文件名）
		 */
		std::wstring GetExecutableDirectory();

		/**
		 * 获取当前工作目录
		 * @return 当前工作目录路径
		 */
		std::wstring GetCurrentWorkingDirectory();

		/**
		 * 获取可执行文件的完整路径（包含文件名）
		 * @return 可执行文件的完整路径
		 */
		std::wstring GetExecutablePath();
	};
};