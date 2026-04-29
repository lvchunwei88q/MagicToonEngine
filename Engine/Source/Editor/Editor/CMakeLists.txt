include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(Editor)

# 添加库
add_library(Editor SHARED
    ${Editor_SOURCES}
    ${Editor_HEADERS}
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(Editor PRIVATE EDITOR_EXPORTS)

target_link_libraries(Editor PRIVATE Core)
target_link_libraries(Editor PRIVATE Logger)

target_link_libraries(Editor PRIVATE RenderCore)
target_link_libraries(Editor PRIVATE RenderUI)
target_link_libraries(Editor PUBLIC EditorWindows)