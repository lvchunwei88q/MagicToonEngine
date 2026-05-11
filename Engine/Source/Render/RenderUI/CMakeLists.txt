include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderUI)

# 添加库
add_engine_library(RenderUI SHARED
    ${RenderUI_SOURCES}
    ${RenderUI_HEADERS}
)

target_include_directories(RenderUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用反射库 使用Lua 库与Sol库
)

set(BIND_SOURCES # 单独设置
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/BindLua/MagicUI_BindCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/BindLua/MagicUI_BindLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/BindLua/MagicUI_BindTableDraw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/BindLua/MagicUI_BindUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Private/BindLua/MagicUI_BindWindowPopup.cpp
)

set_source_files_properties(${BIND_SOURCES} PROPERTIES
    COMPILE_FLAGS "$<$<CXX_COMPILER_ID:MSVC>:/MP>"
)

target_compile_definitions(RenderUI PRIVATE RENDERUI_EXPORTS)

target_link_libraries(RenderUI PRIVATE RenderCore)
target_link_libraries(RenderUI PUBLIC RenderRT)

target_link_libraries(RenderUI PRIVATE Logger)
target_link_libraries(RenderUI PRIVATE IO)

target_link_libraries(RenderUI PRIVATE IMGUI)
target_link_libraries(RenderUI PRIVATE liblua)

target_link_libraries(RenderUI PRIVATE RuntimeAssets)
