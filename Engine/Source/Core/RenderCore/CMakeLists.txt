include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RENDERCORE)

# 添加库
add_library(RenderCore SHARED
    ${RENDERCORE_SOURCES}
    ${RENDERCORE_HEADERS}
)

target_include_directories(RenderCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RenderCore PRIVATE RENDERCORE_EXPORTS)

target_link_libraries(RenderCore PRIVATE Core)
target_link_libraries(RenderCore PRIVATE d3d11)
target_link_libraries(RenderCore PRIVATE dxgi)