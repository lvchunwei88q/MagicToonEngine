#include "MagicUtilities.h"
#include "Common/Checker.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <chrono>

// Core Libraries
#include <IO.h>
#include <Jobsystem/JobSystem.h>

namespace fs = std::filesystem;

namespace Utilitie {
    size_t GetFileModificationTime(const std::wstring& filepath)
    {
        if (!fs::exists(filepath)) {
            return 0;
        }
        auto ftime = fs::last_write_time(filepath);
        // C 17 Method: Convert to time_t
        auto sys_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
        );

        return static_cast<size_t>(std::chrono::system_clock::to_time_t(sys_time));
    }

    bool ReadFilesParallel(const std::vector<std::wstring>& filePaths, std::vector<std::string>& contents) {
        contents.resize(filePaths.size());
        std::vector<std::string> errors(filePaths.size());

        Core::JobSystem::Get().ParallelFor(
            static_cast<uint32_t>(filePaths.size()),
            [&](uint32_t index) {
                std::ifstream file(filePaths[index], std::ios::binary);
                if (!file) {
                    errors[index] = "Failed to open file: " + IO::ToNarrowString(filePaths[index]);
                    return;
                }

                file.seekg(0, std::ios::end);
                const std::streampos end = file.tellg();
                if (end < 0) {
                    errors[index] = "Failed to determine file size: " + IO::ToNarrowString(filePaths[index]);
                    return;
                }

                const auto size = static_cast<std::streamsize>(end);
                file.seekg(0, std::ios::beg);
                if (!file) {
                    errors[index] = "Failed to seek file: " + IO::ToNarrowString(filePaths[index]);
                    return;
                }

                contents[index].resize(static_cast<size_t>(size));
                if (size > 0) {
                    file.read(contents[index].data(), size);
                    if (!file) {
                        errors[index] = "Failed to read file: " + IO::ToNarrowString(filePaths[index]);
                    }
                }
            }
        );

        bool succeeded = true;
        for (const std::string& error : errors) {
            if (!error.empty()) {
                Log::Error(error);
                succeeded = false;
            }
        }
        return succeeded;
    }
} // namespace IO
