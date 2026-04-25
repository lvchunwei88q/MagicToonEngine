#include <AbsolutePath.h>
#include <windows.h>
#include <filesystem>

namespace IO {

    std::wstring AbsolutePath::GetExecutableDirectory()
    {
        std::filesystem::path exePath = GetExecutablePath();
        return exePath.parent_path().wstring();
    }

    std::wstring AbsolutePath::GetCurrentWorkingDirectory()
    {
        return std::filesystem::current_path().wstring();
    }

    std::wstring AbsolutePath::GetExecutablePath()
    {
        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(NULL, buffer, MAX_PATH);
        if (length == 0 || length == MAX_PATH) {
            return L"";
        }
        return std::filesystem::path(buffer).wstring();
    }
}