include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EditorWindows)

# 添加库
add_library(EditorWindows SHARED
    ${EditorWindows_SOURCES}
    ${EditorWindows_HEADERS}
)

target_include_directories(EditorWindows
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用反射库
)

target_compile_definitions(EditorWindows PRIVATE EDITORWINDOWS_EXPORTS)

target_link_libraries(EditorWindows PRIVATE Core)
target_link_libraries(EditorWindows PRIVATE IO)
target_link_libraries(EditorWindows PRIVATE Logger)

target_link_libraries(EditorWindows PRIVATE RenderCore)

target_link_libraries(EditorWindows PRIVATE dwmapi.lib)