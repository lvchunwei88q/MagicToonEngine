include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(Render)

# 添加库
add_engine_library(Render SHARED
    ${Render_SOURCES}
    ${Render_HEADERS}
)

target_include_directories(Render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(Render PRIVATE RENDER_EXPORTS)

target_link_libraries(Render PRIVATE Core)
target_link_libraries(Render PRIVATE RenderCore)