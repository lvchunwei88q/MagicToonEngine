// Core/Private/LuaVM/LuaVMImpl.h
#pragma once

#include <lua.hpp>

#include "LuaVM.h"
#include <string>
#include <unordered_map>

namespace Core {
    namespace {
        template<typename T>
        void PushValueToLua(lua_State* L, const T& value) {
            if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int>) {
                lua_pushinteger(L, static_cast<lua_Integer>(value));
            }
            else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
                lua_pushnumber(L, static_cast<lua_Number>(value));
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(L, value.c_str());
            }
            else {
                // Compile-time error: unsupported type
                static_assert(std::is_same_v<T, void>, "Unsupported type for SetGlobal");
            }
        }
    }

    // Responsible for allocating memory for Lua's virtual machine
    class LuaAllocator {
    public:
        LuaAllocator() = default;

        //Lua allocator callback function
        static void* Allocate(void* ud, void* ptr, size_t osize, size_t nsize);
        //Get the current total allocated bytes
        size_t GetTotalAllocated() const { return m_totalAllocated; }
    private:
        size_t m_totalAllocated{0};
    };

    // Implementation class that holds all Lua states
    class LuaVM::Impl {
    public:
        Impl();
        ~Impl();

        // Implementation of a public interface
        bool DoFile(const std::string& filePath, std::string* errorMsg);
        bool DoString(const std::string& chunk, std::string* errorMsg);
        void RegisterFunction(const std::string& name, int (*func)(lua_State*));

        /////////////////////////// Global Variable
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

        // Get underlying state (for internal use only)
        lua_State* GetState() const { return L; }
        LuaAllocator Allocator;
    private:
        lua_State* L;  // Actual Lua State Machine
        bool ExecuteLoadedFunction(int numArgs, int numResults, std::string* errorMsg);
        std::string PopErrorMessage();

        // Simplify array registration using templates
        template<typename T>
        void SetGlobalVector(const std::string& name, const std::vector<T>& value) {
            lua_newtable(L); // Create a new table
            for (size_t i = 0; i < value.size(); ++i) {
                // Lua Table indexes start from 1
                lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
                PushValueToLua(L, value[i]);
                lua_settable(L, -3);// Store the key-value pair in the table
            }
            // Finally assign it to the global variable
            lua_setglobal(L, name.c_str());
        }
    };

} // namespace Core