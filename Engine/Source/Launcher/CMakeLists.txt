include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(LAUNCHER)

# 添加库
add_executable(Launcher
    ${LAUNCHER_SOURCES}
    ${LAUNCHER_HEADERS}
)

target_sources(Launcher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(Launcher PRIVATE 
    Core
    Editor
)