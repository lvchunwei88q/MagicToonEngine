include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EngineLauncher)

# 添加库
add_executable(EngineLauncher
    ${EngineLauncher_SOURCES}
    ${EngineLauncher_HEADERS}
)

target_sources(EngineLauncher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(EngineLauncher PRIVATE 
    Core
    Editor
    EngineProject
)