include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(LAUNCHER)

# 添加库
add_executable(Launcher
    ${LAUNCHER_SOURCES}
    ${LAUNCHER_HEADERS}
)

target_link_libraries(Launcher PRIVATE Core)
target_link_libraries(Launcher PRIVATE EditorWindows)