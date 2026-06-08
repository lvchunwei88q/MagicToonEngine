include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicBuildTool)

# 添加库
add_executable(MagicBuildTool 
    ${MagicBuildTool_SOURCES}
    ${MagicBuildTool_HEADERS}
)

target_include_directories(MagicBuildTool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(MagicBuildTool PRIVATE MBT_EXPORTS)

target_link_libraries(MagicBuildTool PRIVATE Core)
target_link_libraries(MagicBuildTool PRIVATE Logger)