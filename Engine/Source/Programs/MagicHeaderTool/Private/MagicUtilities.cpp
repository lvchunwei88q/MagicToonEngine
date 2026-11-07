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

    std::vector<std::string> ReadFilesParallel(const std::vector<std::wstring>& filePaths) {
        std::vector<std::string> contents(filePaths.size());

        // Using JobSystem's ParallelFor to read files in parallel
        Core::JobSystem::Get().ParallelFor(
            static_cast<uint32_t>(filePaths.size()),
            [&](uint32_t index) {
                std::ifstream file(filePaths[index], std::ios::binary);
                if (file.is_open()) {
                    // Get the size of the file
                    file.seekg(0, std::ios::end);
                    size_t size = static_cast<size_t>(file.tellg());
                    file.seekg(0, std::ios::beg);

                    contents[index].resize(size);
                    file.read(&contents[index][0], size);
                }
                else {
                    ThrowErrorMessage("Failed to open file: " + IO::ToNarrowString(filePaths[index]));
                }
            }
        );
        return contents;
    }
} // namespace IO