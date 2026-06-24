#include "IO.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;
namespace IO {

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

    std::vector<uint8_t> ReadAllU8Bytes(const std::wstring& path)
    {
        std::vector<char> Content = ReadAllBytes(path);
        std::vector<uint8_t> uint8_content(Content.begin(), Content.end());
        return uint8_content;
    }

    void WriteAllText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void WriteAllBytes(const std::wstring& path, const BinaryWrite& data) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + ToNarrowString(path));
        }
        file.write(data.DataChar(), data.GetUsedSize());
    }

    void AppendText(const std::wstring& path, const std::string& content) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
        }
        file.write(content.data(), content.size());
    }

    void AppendBytes(const std::wstring& path, const BinaryWrite& data) {
        fs::path p(path);
        std::ofstream file(p, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + ToNarrowString(path));
        }
        file.write(data.DataChar(), data.GetUsedSize());
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
            fs::remove_all(p, ec);  // remove all content
            return !ec;
        }
        return false;  // Not a folder, return false
    }

    bool CreateFile(const std::wstring& path) {
        fs::path p(path);
        if (fs::exists(p)) {
            // Update Last Modified Time
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

            // No, if the target already exists and is a file, delete it first
            if (fs::exists(dstPath) && fs::is_regular_file(dstPath)) {
                fs::remove(dstPath);
            }

            // Move/Rename
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

    bool GetFileName(const std::wstring& src, std::wstring& dst)
    {
        try {
            std::filesystem::path path(src);
            // Get FileName
            std::filesystem::path filename = path.filename();
            // Remove the extension
            std::filesystem::path stem = filename.stem();

            dst = stem.wstring();
            return !dst.empty();
        }
        catch (const std::filesystem::filesystem_error&) {
            dst.clear();
            return false;
        }
    }

    bool CreateDirectory(const std::wstring& path) {
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

} // namespace IO