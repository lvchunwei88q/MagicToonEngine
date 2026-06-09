#pragma once

#include "Common/Compiler.h"
#include "Tools/Singleton.h"
#include <string>
#include <vector>

DISABLE_DLL_WARNINGS_PUSH;

namespace IO {
	// 并行读取文件
	std::vector<std::string> ReadFilesParallel(const std::vector<std::wstring>& filePaths);

	// 读取整个文件（文本模式，返回字符串）
	std::string ReadAllText(const std::wstring& path);

	// 读取整个文件（二进制模式，返回字节向量）
	std::vector<char> ReadAllBytes(const std::wstring& path);

	// 写入字符串到文件（覆盖）
	void WriteAllText(const std::wstring& path, const std::string& content);

	// 写入字节到文件（覆盖）
	void WriteAllBytes(const std::wstring& path, const std::vector<char>& data);

	// 追加字符串到文件末尾
	void AppendText(const std::wstring& path, const std::string& content);

	// 追加字节到文件末尾
	void AppendBytes(const std::wstring& path, const std::vector<char>& data);

	// 删除文件
	bool DeleteToFile(const std::wstring& path);

	// 删除文件夹
	bool DeleteToDirectory(const std::wstring& path);

	// 创建空文件（如果文件已存在则更新其时间戳）
	bool MakeFile(const std::wstring& path);

	// 移动/重命名文件或目录
	bool MoveFile(const std::wstring& src, const std::wstring& dst);

	// 获取文件的目录
	bool GetFileDirectory(const std::wstring& src, std::wstring& dst);

	// 创建目录（包括父目录）
	bool MakeDirectory(const std::wstring& path);

	// 检查文件或目录是否存在
	bool Exists(const std::wstring& path);

	// 扫描指定目录下的所有文件（不含子目录），返回文件名列表
	std::vector<std::string> GetFilesInDirectory(const std::wstring& path);

	// 窄字符转宽字符 (UTF-8 to UTF-16)
	std::wstring ToWideString(const std::string& narrow);
	// 宽字符转窄字符 (UTF-16 to UTF-8)
	std::string ToNarrowString(const std::wstring& wide);
}

DISABLE_DLL_WARNINGS_POP;