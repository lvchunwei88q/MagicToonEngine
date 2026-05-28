include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicLauncher)

# 添加库
add_executable(MagicLauncher WIN32 
    ${MagicLauncher_SOURCES}
    ${MagicLauncher_HEADERS}
)

target_include_directories(MagicLauncher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用WebView2JSON HTTP 库 
        ${CMAKE_SOURCE_DIR}/Engine/Content # 内容目录
)

target_sources(MagicLauncher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(MagicLauncher PRIVATE # windows
    user32
    dwmapi
)

target_link_libraries(MagicLauncher PRIVATE Core)
target_link_libraries(MagicLauncher PRIVATE EngineProject)
target_link_libraries(MagicLauncher PRIVATE RenderCore)
target_link_libraries(MagicLauncher PRIVATE RenderLauncher)