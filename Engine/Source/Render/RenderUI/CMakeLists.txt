include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderUI)

# 添加库
add_library(RenderUI SHARED
    ${RenderUI_SOURCES}
    ${RenderUI_HEADERS}
)

target_include_directories(RenderUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RenderUI PRIVATE RENDERUI_EXPORTS)

target_link_libraries(RenderUI PRIVATE RenderCore)
target_link_libraries(RenderUI PUBLIC RenderRT)
target_link_libraries(RenderUI PRIVATE IMGUI)