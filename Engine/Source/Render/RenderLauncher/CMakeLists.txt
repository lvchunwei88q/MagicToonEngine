include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderLauncher)

# 添加库
add_engine_library(RenderLauncher SHARED
    ${RenderLauncher_SOURCES}
    ${RenderLauncher_HEADERS}
)

target_include_directories(RenderLauncher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
        ${THIRD_PARTY_DIR} # 使用反射库
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RenderLauncher PRIVATE RENDERLAUNCHER_EXPORTS)


target_link_libraries(RenderLauncher PRIVATE RenderUI)
target_link_libraries(RenderLauncher PRIVATE Logger)
target_link_libraries(RenderLauncher PRIVATE IO)
target_link_libraries(RenderLauncher PRIVATE RuntimeAssets)
