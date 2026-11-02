include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderLauncher)

# add library
add_engine_library(RenderLauncher SHARED
    ${RenderLauncher_SOURCES}
    ${RenderLauncher_HEADERS}
)

target_include_directories(RenderLauncher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR} # 使用反射库
)

target_compile_definitions(RenderLauncher PRIVATE RENDERLAUNCHER_EXPORTS)


target_link_libraries(RenderLauncher PRIVATE RenderUI)
target_link_libraries(RenderLauncher PRIVATE Logger)
target_link_libraries(RenderLauncher PRIVATE EngineProject)

target_link_libraries(RenderLauncher PUBLIC RenderRT)
