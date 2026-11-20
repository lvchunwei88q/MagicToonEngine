#include "IO.h"
#include <fstream>
#include <stdexcept>

namespace IO {
    namespace {
        void WriteAllBytesImpl(const path& filePath, const void* data, size_t size) {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to write file: " + filePath.string());
            }
            if (size > 0) {
                file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
                if (!file) {
                    throw std::runtime_error("Failed to write data to file: " + filePath.string());
                }
            }
        }

        void AppendBytesImpl(const path& filePath, const void* data, size_t size) {
            std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::app);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to append to file: " + filePath.string());
            }
            if (size > 0) {
                file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
                if (!file) {
                    throw std::runtime_error("Failed to append data to file: " + filePath.string());
                }
            }
        }
    } // namespace

    std::string ReadAllText(const path& filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<char> ReadAllCharBytes(const path& filePath) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }
        std::vector<char> content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }

    std::vector<uint8_t> ReadAllUINT8Bytes(const path& filePath) {
        std::vector<char> Content = ReadAllCharBytes(filePath);
        std::vector<uint8_t> uint8_content(Content.begin(), Content.end());
        return uint8_content;
    }

    BinaryReader ReadAllBinaryReaderBytes(const path& filePath)
    {
        std::vector<char> Content = ReadAllCharBytes(filePath);
        std::vector<uint8_t> uint8_content(Content.begin(), Content.end());
        return BinaryReader(uint8_content);
    }

    void WriteAllText(const path& filePath, const std::string& content) {
        std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to write file: " + filePath.string());
        }
        file.write(content.data(), content.size());
    }

    void WriteAllBytes(const path& filePath, const std::vector<char>& data) {
        WriteAllBytesImpl(filePath, data.data(), data.size());
    }

    void WriteAllBytes(const path& filePath, const std::vector<uint8_t>& data) {
        WriteAllBytesImpl(filePath, data.data(), data.size());
    }

    void WriteAllBytes(const path& filePath, const BinaryWriter& data) {
        WriteAllBytesImpl(filePath, data.DataU8(), data.GetWrittenSize());
    }

    void WriteAllBytes(const path& filePath, const uint8_t* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("WriteAllBytes: null data pointer");
        }
        WriteAllBytesImpl(filePath, data, size);
    }

    void WriteAllBytes(const path& filePath, const char* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("WriteAllBytes: null data pointer");
        }
        WriteAllBytesImpl(filePath, data, size);
    }

    void AppendText(const path& filePath, const std::string& content) {
        std::ofstream file(filePath, std::ios::out | std::ios::binary | std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to append to file: " + filePath.string());
        }
        file.write(content.data(), content.size());
    }

    void AppendBytes(const path& filePath, const std::vector<char>& data) {
        AppendBytesImpl(filePath, data.data(), data.size());
    }

    void AppendBytes(const path& filePath, const std::vector<uint8_t>& data) {
        AppendBytesImpl(filePath, data.data(), data.size());
    }

    void AppendBytes(const path& filePath, const BinaryWriter& data) {
        AppendBytesImpl(filePath, data.DataU8(), data.GetWrittenSize());
    }

    void AppendBytes(const path& filePath, const uint8_t* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("AppendBytes: null data pointer");
        }
        AppendBytesImpl(filePath, data, size);
    }

    void AppendBytes(const path& filePath, const char* data, size_t size) {
        if (size > 0 && data == nullptr) {
            throw std::invalid_argument("AppendBytes: null data pointer");
        }
        AppendBytesImpl(filePath, data, size);
    }

    bool DeleteToFile(const path& filePath) {
        if (fs::is_regular_file(filePath)) {
            return fs::remove(filePath);
        }
        return false;
    }

    bool DeleteToDirectory(const path& directoryPath) {
        std::error_code ec;

        if (fs::is_directory(directoryPath)) {
            fs::remove_all(directoryPath, ec);  // remove all content
            return !ec;
        }
        return false;  // Not a folder, return false
    }

    bool CreateFile(const path& filePath) {
        if (fs::exists(filePath)) {
            // Update Last Modified Time
            fs::last_write_time(filePath, fs::file_time_type::clock::now());
            return true;
        }
        std::ofstream file(filePath, std::ios::out | std::ios::binary);
        return file.is_open();
    }

    bool MoveFile(const path& src, const path& dst) {
        try {
            // No, if the target already exists and is a file, delete it first
            if (fs::exists(dst) && fs::is_regular_file(dst)) {
                fs::remove(dst);
            }

            // Move/Rename
            fs::rename(src, dst);
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool GetFileDirectory(const path& src, path& dst)
    {
        dst = src.parent_path();
        return !dst.empty();
    }

    bool GetFileName(const path& src, path& dst)
    {
        try {
            // Get FileName
            path filename = src.filename();
            // Remove the extension
            path stem = filename.stem();

            dst = stem;
            return !dst.empty();
        }
        catch (const std::filesystem::filesystem_error&) {
            dst.clear();
            return false;
        }
    }

    bool CreateDirectory(const path& directoryPath) {
        std::error_code error;
        fs::create_directories(directoryPath, error);
        if (!error) {
            return true;
        }

        std::error_code statusError;
        return fs::is_directory(directoryPath, statusError) && !statusError;
    }

    bool Exists(const path& filePath) {
        return fs::exists(filePath);
    }

    std::vector<std::string> GetFilesInDirectory(const path& directoryPath) {
        std::vector<std::string> files;
        if (!fs::is_directory(directoryPath)) {
            return files;
        }
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry)) {
                files.push_back(entry.path().filename().string());
            }
        }
        return files;
    }

} // namespace IO