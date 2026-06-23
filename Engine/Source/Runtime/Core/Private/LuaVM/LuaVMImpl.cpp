// Core/Private/LuaVM/LuaVMImpl.cpp
#include "LuaVMImpl.h"
#include "IO.h"

namespace Core {

    namespace {
        auto RegisterSaveRef = [](LuaScriptRegisterStatus& Rs, lua_State* Lua) -> bool {
            // If Register is true, it means it's already registered, so you can't register it again.
            if (Rs.Register) {
                WarningCapture::Capture("Please release the current Lua bytecode before loading other Lua code!");
                return false;
            }
            Rs.Ref = luaL_ref(Lua, LUA_REGISTRYINDEX);
            // We need to check if the return value is valid
            if (Rs.Ref == LUA_NOREF || Rs.Ref == LUA_REFNIL) {
                Rs.Register = false;
                Rs.Ref = LUA_NOREF;
                WarningCapture::Capture("Failed to register Lua function (invalid or nil)");
                return false;
            }
            Rs.Register = true;
            return true;
        };
    }

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
        Lua = luaL_newstate(LuaAllocator::Allocate, static_cast<void*>(&Allocator));
        if (!Lua) {
            throw std::runtime_error("LuaVM: Failed to create lua_State");
        }

        luaL_openlibs(Lua);
    }

    LuaVM::Impl::~Impl() {
        if (Lua) {
            lua_close(Lua);
            Lua = nullptr;
        }
    }

    bool LuaVM::Impl::LoadFile(const std::string& filePath, std::string* errorMsg) {
        if (luaL_loadfile(Lua, filePath.c_str()) != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(Lua, 1);
            return false;
        }
        // Save Reference ID
        return RegisterSaveRef(this->Rs, Lua);
    }

    bool LuaVM::Impl::LoadString(const std::string& chunk, std::string* errorMsg) {
        if (luaL_loadstring(Lua, chunk.c_str()) != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(Lua, 1);
            return false;
        }
        // Save Reference ID
        return RegisterSaveRef(this->Rs, Lua);
    }

    bool LuaVM::Impl::Execute(int numArgs, int numResults, LuaResultCallback callback, std::string* errorMsg)
    {
        // Return immediately if not registered
        if (!Rs.Register) {
            if (errorMsg) *errorMsg = "No Lua function loaded. Call LoadString/LoadFile first.";
            return false;
        }
        // Pop a function from the stack using its ID
        lua_rawgeti(Lua, LUA_REGISTRYINDEX, Rs.Ref);

        // Check whether the top of the stack is a function (safety check)
        if (!lua_isfunction(Lua, -1)) {
            if (errorMsg) *errorMsg = "Registered reference is not a function.";
            lua_pop(Lua, 1);
            return false;
        }

        int status = lua_pcall(Lua, numArgs, numResults, 0);
        if (status != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(Lua, 1);
            return false;
        }
        // Execution successful: numResults return values are on the top of the stack
        int nresults = lua_gettop(Lua);
        if (callback)
            callback(Lua, nresults);
        lua_pop(Lua, nresults);

        // Note: The function has been popped, Rs.Ref is still valid, and can be executed again
        return true;
    }

    void LuaVM::Impl::Unload()
    {
        if (!Rs.Register) {
            WarningCapture::Capture("Lua bytecode Already uninstalled!");
            return;  // Already uninstalled
        }
        // Release the reference in the registry
        luaL_unref(Lua, LUA_REGISTRYINDEX, Rs.Ref);
        Rs.Ref = LUA_NOREF;
        // Clear the stack
        lua_settop(Lua, 0);

        Rs.Register = false;
    }

    bool LuaVM::Impl::RegisterPackagePath(const std::string& path)
    {
        // First, build the Lua source code and then compile it.
        // Hand it over to the Lua VM to execute, and this way you can modify global variables located in the global state.
        std::string errormsg;
        std::string script =
            "package.path = package.path .. \";" +
            path +
            "/?.lua\"";
        // Load Lua (Load and Compile Lua)
        bool result = LoadString(script, &errormsg);
        if (result) {
            result = Execute(0, LUAVM_MULTRET,nullptr, &errormsg);
        }
        // Whether it succeeds or not, we'll clear it once anyway.
        Unload();
        
        if (!result) {
            ErrorCapture::Capture(errormsg);
            return false;
        }
        return true;
    }

    void LuaVM::Impl::RegisterFunction(const std::string& name,lua_CFunction func) {
        lua_register(Lua, name.c_str(), func);
    }

    /////////////////////////// Global Variable
    void LuaVM::Impl::SetGlobal(const std::string& name, int value) {
        lua_pushinteger(Lua, value);
        lua_setglobal(Lua, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, float value) {
        lua_pushnumber(Lua, static_cast<lua_Number>(value));
        lua_setglobal(Lua, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, double value) {
        lua_pushnumber(Lua, value);
        lua_setglobal(Lua, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, bool value) {
        lua_pushboolean(Lua, value ? 1 : 0);
        lua_setglobal(Lua, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const char* value) {
        lua_pushstring(Lua, value ? value : "");
        lua_setglobal(Lua, name.c_str());
    }

    void LuaVM::Impl::SetGlobal(const std::string& name, const std::string& value) {
        lua_pushstring(Lua, value.c_str());
        lua_setglobal(Lua, name.c_str());
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
        int status = lua_pcall(Lua, numArgs, numResults, 0);
        if (status != LUA_OK) {
            if (errorMsg) *errorMsg = PopErrorMessage();
            else lua_pop(Lua, 1);
            return false;
        }
        return true;
    }

    std::string LuaVM::Impl::PopErrorMessage() {
        // When lua_pcall fails, the error message is pushed onto the top of the stack
        const char* err = lua_tostring(Lua, -1);
        std::string result = err ? err : "Unknown error";
        lua_pop(Lua, 1);
        return result;
    }

    // ---- LuaVM Public Interface (Proxy) ----

    LuaVM::LuaVM() : pImpl(std::make_unique<Impl>()) {}
    LuaVM::~LuaVM() = default;

    // ------------------------------- Core ----------------------------------- //

    bool LuaVM::LoadFile(const std::string& filePath, std::string* errorMsg) {
        return pImpl->LoadFile(filePath, errorMsg);
    }

    bool LuaVM::LoadString(const std::string& chunk, std::string* errorMsg) {
        return pImpl->LoadString(chunk, errorMsg);
    }

    bool LuaVM::Execute(int numArgs, int numResults, LuaResultCallback callback, std::string* errorMsg){
        return pImpl->Execute(numArgs, numResults, callback, errorMsg);
    }

    void LuaVM::Unload(){
        pImpl->Unload();
    }

    bool LuaVM::IsLoaded() const{
        // Whether to register here means whether to read it 
        return pImpl->GetRegisterStatus().Register;
    }

    bool LuaVM::RegisterPackagePath(const std::string& path) {
        return pImpl->RegisterPackagePath(path);
    }

    void LuaVM::RegisterFunction(const std::string& name, const LuaCFunction& func) {
        pImpl->RegisterFunction(name, func);
    }

    // ------------------------------- Core End ----------------------------------- //

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