include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(Lua)

add_engine_external_library(Lua SHARED
    ${Lua_SOURCES}
    ${Lua_HEADERS}
)

target_compile_definitions(Lua PRIVATE LUA_EXPORTS)
target_include_directories(Lua
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal
)