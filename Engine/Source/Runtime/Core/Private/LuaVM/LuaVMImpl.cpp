// Core/Private/LuaVM/LuaVMImpl.cpp
#include "LuaVMImpl.h"

namespace Core {

    void* LuaAllocator::Allocate(void* ud, void* ptr, size_t osize, size_t nsize) {
        // We need to use our own memory allocator, so we have to pass a pointer to our allocator in the user data.
        LuaAllocator* allocator = static_cast<LuaAllocator*>(ud);

        // In Lua, allocating a size of 0 means you need to delete this area, so we delete this memory.
        if (nsize == 0) {
            // free
            free(ptr);
            return nullptr;
        }

        if (ptr == nullptr) {
            // new malloc
            void* result = malloc(nsize);
            if (result && allocator) {
                allocator->m_totalAllocated += nsize;
            }
            return result;
        }

        // realloc
        void* result = realloc(ptr, nsize);
        if (result && allocator) {
            // First subtract the old size, then add the new size
            allocator->m_totalAllocated -= osize;
            allocator->m_totalAllocated += nsize;
        }
        return result;
    }

    // ---- Impl achieve ----

    LuaVM::Impl::Impl() {
        /* This operation will create a new Lua global state,
        *  and afterwards we can directly create Lua state threads without needing to call luaL_newstate.
        */

        // Using our own memory allocator
        L = luaL_newstate(LuaAllocator::Allocate, static_cast<void*>(&Allocator));
        if (!L) {
            throw std::runtime_error("LuaVM: Failed to create lua_State");
        }
        luaL_openlibs(L);
    }

    LuaVM::Impl::~Impl() {
        if (L) {
            lua_close(L);
            L = nullptr;
        }
    }

    bool LuaVM::Impl::DoFile(const std::string& filePath, std::string* errorMsg) {
        if (luaL_loadfile(L, filePath.c_str()) != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(L, 1);
            return false;
        }
        return ExecuteLoadedFunction(0, LUA_MULTRET, errorMsg);
    }

    bool LuaVM::Impl::DoString(const std::string& chunk, std::string* errorMsg) {
        if (luaL_loadstring(L, chunk.c_str()) != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(L, 1);
            return false;
        }
        return ExecuteLoadedFunction(0, LUA_MULTRET, errorMsg);
    }

    void LuaVM::Impl::RegisterFunction(const std::string& name, int (*func)(lua_State*)) {
        lua_register(L, name.c_str(), func);
    }

    /////////////////////////// Global Variable
    void LuaVM::Impl::SetGlobal(const std::string& name, int value) {
        lua_pushinteger(L, value);
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, float value) {
        lua_pushnumber(L, static_cast<lua_Number>(value));
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, double value) {
        lua_pushnumber(L, value);
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, bool value) {
        lua_pushboolean(L, value ? 1 : 0);
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const char* value) {
        lua_pushstring(L, value ? value : "");
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::string& value) {
        lua_pushstring(L, value.c_str());
        lua_setglobal(L, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::vector<uint8_t>& value) {
        SetGlobalVector(name, value);
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::vector<int>& value) {
        SetGlobalVector(name, value);
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::vector<float>& value) {
        SetGlobalVector(name, value);
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::vector<std::string>& value) {
        SetGlobalVector(name, value);
    }
    ///////////////////////////

    bool LuaVM::Impl::ExecuteLoadedFunction(int numArgs, int numResults, std::string* errorMsg) {
        int status = lua_pcall(L, numArgs, numResults, 0);
        if (status != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(L, 1);
            return false;
        }
        return true;
    }

    std::string LuaVM::Impl::PopErrorMessage() {
        // When lua_pcall fails, the error message is pushed onto the top of the stack
        const char* err = lua_tostring(L, -1);
        std::string result = err ? err : "Unknown error";
        lua_pop(L, 1);
        return result;
    }

    // ---- LuaVM Public Interface (Proxy) ----

    LuaVM::LuaVM() : pImpl(std::make_unique<Impl>()) {}
    LuaVM::~LuaVM() = default;

    bool LuaVM::DoFile(const std::string& filePath, std::string* errorMsg) {
        return pImpl->DoFile(filePath, errorMsg);
    }

    bool LuaVM::DoString(const std::string& chunk, std::string* errorMsg) {
        return pImpl->DoString(chunk, errorMsg);
    }

    void LuaVM::RegisterFunction(const std::string& name, int (*func)(lua_State*)) {
        pImpl->RegisterFunction(name, func);
    }

    void LuaVM::SetGlobal(const std::string& name, int value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, float value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, double value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, bool value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const std::string& value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const char* value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const std::vector<uint8_t>& value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const std::vector<int>& value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const std::vector<float>& value) {
        pImpl->SetGlobal(name, value);
    }

    void LuaVM::SetGlobal(const std::string& name, const std::vector<std::string>& value) {
        pImpl->SetGlobal(name, value);
    }
} // namespace Core