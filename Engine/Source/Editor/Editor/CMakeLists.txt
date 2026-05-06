include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(Editor)

# 添加库
add_engine_library(Editor SHARED
    ${Editor_SOURCES}
    ${Editor_HEADERS}
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用反射库
        ${CMAKE_SOURCE_DIR}/Engine/Content
)

target_compile_definitions(Editor PRIVATE EDITOR_EXPORTS)

target_link_libraries(Editor PUBLIC Core)
target_link_libraries(Editor PRIVATE IO)
target_link_libraries(Editor PRIVATE Logger)

target_link_libraries(Editor PRIVATE RenderCore)
target_link_libraries(Editor PRIVATE RenderRT)
target_link_libraries(Editor PRIVATE RenderUI)

target_link_libraries(Editor PRIVATE dwmapi.lib)