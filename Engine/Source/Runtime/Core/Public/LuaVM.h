#pragma once

#include "CoreMinimal.h"
#include "Common/Check.h"

#include <string>
#include <functional>
#include <memory>

// Forward declaration, hiding the specific implementation
struct lua_State;

DISABLE_DLL_WARNINGS_PUSH
namespace Core {
    /**
     * LuaVM - Lua virtual machine
     */
    class CORE_API LuaVM {
    public:
        LuaVM();
        ~LuaVM();

        // No copying
        LuaVM(const LuaVM&) = delete;
        LuaVM& operator=(const LuaVM&) = delete;

        // ---- Core ----
        bool DoFile(const std::string& filePath, std::string* errorMsg = nullptr);
        bool DoString(const std::string& chunk, std::string* errorMsg = nullptr);

        // ---- C++ Lua Registration Operation ----
        bool RegisterPackagePath(const std::string& path);
        void RegisterFunction(const std::string& name, int (*func)(lua_State*));

        // ---- Global variable operations ----
        void SetGlobal(const std::string& name, int value);
        void SetGlobal(const std::string& name, float value);
        void SetGlobal(const std::string& name, double value);
        void SetGlobal(const std::string& name, bool value);
        void SetGlobal(const std::string& name, const char* value);
        void SetGlobal(const std::string& name, const std::string& value);

        void SetGlobal(const std::string& name, const std::vector<uint8_t>& value);
        void SetGlobal(const std::string& name, const std::vector<int>& value);
        void SetGlobal(const std::string& name, const std::vector<float>& value);
        void SetGlobal(const std::string& name, const std::vector<std::string>& value);

    private:
        // There's only one opaque pointer here, pointing to the actual implementation object.
        class Impl;
        std::unique_ptr<Impl> pImpl;
    };
}
DISABLE_DLL_WARNINGS_POP