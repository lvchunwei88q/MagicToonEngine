include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderRT)

# 添加库
add_engine_library(RenderRT SHARED
    ${RenderRT_SOURCES}
    ${RenderRT_HEADERS}
)

target_include_directories(RenderRT
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RenderRT PRIVATE RENDERRT_EXPORTS)

target_link_libraries(RenderRT PRIVATE Core)
target_link_libraries(RenderRT PRIVATE RenderCore)
target_link_libraries(RenderRT PRIVATE Logger)