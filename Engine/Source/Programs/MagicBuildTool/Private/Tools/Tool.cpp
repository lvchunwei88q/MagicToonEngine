#include "Tools/Tool.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "Tools/JobSystem.h"

namespace fs = std::filesystem;
namespace TOOL {
    std::vector<std::string> ReadFilesParallel(const std::vector<std::wstring>& filePaths) {
        std::vector<std::string> contents(filePaths.size());

        // 使用 JobSystem 的 ParallelFor 并行读取文件
        MBT::JobSystem::Get().ParallelFor(
            static_cast<uint32_t>(filePaths.size()),
            [&](uint32_t index) {
                // 打开文件
                std::ifstream file(filePaths[index], std::ios::binary);
                if (file.is_open()) {
                    // 获取文件大小
                    file.seekg(0, std::ios::end);
                    size_t size = static_cast<size_t>(file.tellg());
                    file.seekg(0, std::ios::beg);

                    // 预分配空间并读取
                    contents[index].resize(size);
                    file.read(&contents[index][0], size);
                }
                else {
                    throw std::runtime_error("Failed to open file: " + ToNarrowString(filePaths[index]));
                }
            }
        );
        return contents;
    }

    std::string ReadAllText(const std::wstring& path) {
        fs::path p(path);
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + ToNarrowString(path));
        }
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<char> ReadAllBytes(const std::wstring& path) {
        fs::path p(path);
        std::ifstream file(p, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + ToNarrowString(path));
        }
        std::vector<char> content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    void WriteAllText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void WriteAllBytes(const std::wstring& path, const std::vector<char>& data) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
        }
        file.write(data.data(), data.size());
    }

    void AppendText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void AppendBytes(const std::wstring& path, const std::vector<char>& data) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
        }
        file.write(data.data(), data.size());
    }

    bool DeleteToFile(const std::wstring& path) {
        fs::path p(path);
        if (fs::is_regular_file(p)) {
            return fs::remove(p);
        }
        return false;
    }

    bool DeleteToDirectory(const std::wstring& path) {
        fs::path p(path);
        std::error_code ec;

        if (fs::is_directory(p)) {
            fs::remove_all(p, ec);  // 删除文件夹及其所有内容
            return !ec;
        }
        return false;  // 不是文件夹，返回 false
    }

    bool MakeFile(const std::wstring& path) {
        fs::path p(path);
        if (fs::exists(p)) {
            // 更新最后修改时间
            fs::last_write_time(p, fs::file_time_type::clock::now());
            return true;
        }
        std::ofstream file(p, std::ios::out | std::ios::binary);
        return file.is_open();
    }

    bool MoveFile(const std::wstring& src, const std::wstring& dst) {
        try {
            fs::path srcPath(src);
            fs::path dstPath(dst);

            // 如果目标已存在且是文件，先删除
            if (fs::exists(dstPath) && fs::is_regular_file(dstPath)) {
                fs::remove(dstPath);
            }

            // 移动/重命名
            fs::rename(srcPath, dstPath);
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool GetFileDirectory(const std::wstring& src, std::wstring& dst)
    {
        size_t pos = src.find_last_of(L"\\");
        if (pos != std::wstring::npos) {
            dst = src.substr(0, pos);
            return true;
        }
        return false;
    }

    bool MakeDirectory(const std::wstring& path) {
        fs::path p(path);
        if (fs::exists(p)) {
            return fs::is_directory(p);
        }
        return fs::create_directories(p);
    }

    bool Exists(const std::wstring& path) {
        return fs::exists(fs::path(path));
    }

    std::vector<std::string> GetFilesInDirectory(const std::wstring& path) {
        fs::path dir(path);
        std::vector<std::string> files;
        if (!fs::is_directory(dir)) {
            return files;
        }
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (fs::is_regular_file(entry)) {
                files.push_back(entry.path().filename().string());
            }
        }
        return files;
    }

    std::wstring ToWideString(const std::string& narrow) {
        if (narrow.empty()) return std::wstring();

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), NULL, 0);
        std::wstring wide(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), (int)narrow.size(), &wide[0], size_needed);
        return wide;
    }

    std::string ToNarrowString(const std::wstring& wide) {
        if (wide.empty()) return std::string();

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), NULL, 0, NULL, NULL);
        std::string narrow(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &narrow[0], size_needed, NULL, NULL);
        return narrow;
    }

} // namespace IO