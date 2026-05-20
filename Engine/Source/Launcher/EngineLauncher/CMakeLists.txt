include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EngineLauncher)

# 添加库
add_executable(EngineLauncher WIN32 
    ${EngineLauncher_SOURCES}
    ${EngineLauncher_HEADERS}
)

target_include_directories(EngineLauncher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用WebView2JSON HTTP 库 
        ${CMAKE_SOURCE_DIR}/Engine/Content # 内容目录
)

target_sources(EngineLauncher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(EngineLauncher PRIVATE # windows
    user32
    dwmapi
)

target_link_libraries(EngineLauncher PRIVATE Core)
target_link_libraries(EngineLauncher PRIVATE IO)
target_link_libraries(EngineLauncher PRIVATE EngineProject)
target_link_libraries(EngineLauncher PRIVATE RenderCore)
target_link_libraries(EngineLauncher PRIVATE RenderRT)