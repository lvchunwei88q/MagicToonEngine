#include "lua.h"
#include "lauxlib.h"

// If the user doesn’t provide a log output function, we’ll just print directly to the Windows console.
#include <Windows.h>

#define output_debug_view(str,len)                                   \
if (len < 512) {                                                     \
    char buffer[512];                                                \
    snprintf(buffer, sizeof(buffer), "LuaVM %.*s\n", (int)len, str); \
    OutputDebugStringA(buffer);                                      \
}

static lua_magic_log magic_log_func = NULL;

void set_magic_log_func(lua_magic_log func){
    magic_log_func = func;
}

void Lua_LogWrite(const char* s, size_t len) {
    if (len > 0) {
        if (magic_log_func) {
            magic_log_func(s, len, MAGIC_LUA_LOG_INFO);
        }
        else {
            output_debug_view(s, len);
        }
        //lua_writestringerror("%s", msg.c_str());
    }
}

void Lua_ErrorLogWrite(const char* s, size_t len) {
    if (len > 0) {
        if (magic_log_func) {
            magic_log_func(s, len, MAGIC_LUA_LOG_ERROR);
        }
        else {
            output_debug_view(s, len);
        }
    }
}