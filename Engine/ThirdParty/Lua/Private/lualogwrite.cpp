#include "lauxlib.h"

#include <string>      // strlen
#include <stddef.h>      // size_t

extern "C" void Lua_LogWrite(const char* s, size_t len) {
    if (len > 0) {
        std::string msg(s, len);
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
            msg.pop_back();
        if (!msg.empty()){
            //LOG_INFO("LuaScript: ", msg.c_str());
        }

        //lua_writestringerror("%s", msg.c_str());
    }
}

extern "C" void Lua_ErrorLogWrite(const char* s, size_t len) {
    if (len > 0) {
        std::string msg(s, len);
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
            msg.pop_back();
        if (!msg.empty()) {
            //LOG_ERROR("LuaScript: ", msg.c_str());
        }
            
    }
}