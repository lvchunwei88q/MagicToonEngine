#include "MagicUtilities.h"
#include "Common/Checker.h"
#include <fstream>
#include <stdexcept>

// Core Libraries
#include <IO.h>
#include <Jobsystem/JobSystem.h>

namespace fs = std::filesystem;

namespace Utilitie {
    namespace {
        // FNV-1a 64-bit: stable across runs and across rebuilds of this tool, so
        // the cache file stays valid when MagicCodegenTool itself is rebuilt.
        constexpr uint64_t FNV1A_OFFSET_BASIS = 14695981039346656037ULL;
        constexpr uint64_t FNV1A_PRIME        = 1099511628211ULL;
        constexpr size_t   HASH_CHUNK_SIZE    = 64 * 1024;

        uint64_t HashFileContent(const IO::path& filepath)
        {
            std::ifstream file(filepath, std::ios::binary);
            if (!file) {
                return 0;
            }

            std::vector<char> buffer(HASH_CHUNK_SIZE);
            uint64_t hash = FNV1A_OFFSET_BASIS;

            while (file) {
                file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
                const std::streamsize readCount = file.gcount();
                for (std::streamsize i = 0; i < readCount; ++i) {
                    hash ^= static_cast<unsigned char>(buffer[static_cast<size_t>(i)]);
                    hash *= FNV1A_PRIME;
                }
            }

            // A read error (not end-of-file) must not look like a valid hash.
            return file.eof() ? hash : 0;
        }
    }

    uint64_t HashFNV1a64(const std::string_view s)
    {
        uint64_t hash = FNV1A_OFFSET_BASIS;
        for (char c : s)
        {
            hash ^= static_cast<uint64_t>(static_cast<uint8_t>(c));
            hash *= FNV1A_PRIME;
        }
        return hash;
    }

    uint64_t GetFileContentHash(const IO::path& filepath)
    {
        if (!fs::exists(filepath)) {
            return 0;
        }
        return HashFileContent(filepath);
    }

    void GetFileContentHashesParallel(const std::vector<IO::path>& filePaths, std::vector<uint64_t>& hashes)
    {
        hashes.assign(filePaths.size(), 0);
        if (filePaths.empty()) {
            return;
        }

        Core::JobSystem::Get().ParallelFor(
            static_cast<uint32_t>(filePaths.size()),
            [&](uint32_t index) {
                hashes[index] = HashFileContent(filePaths[index]);
            }
        );
    }

    bool ReadFilesParallel(const std::vector<IO::path>& filePaths, std::vector<std::string>& contents) {
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
