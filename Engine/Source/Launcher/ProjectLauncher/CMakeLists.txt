include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(ProjectLauncher)

# 添加库
add_executable(ProjectLauncher WIN32 
    ${ProjectLauncher_SOURCES}
    ${ProjectLauncher_HEADERS}
)

target_include_directories(ProjectLauncher
    PRIVATE
        ${THIRD_PARTY_DIR} # 使用WebView2库
        ${CMAKE_SOURCE_DIR}/Engine/Content # 内容目录
)

target_sources(ProjectLauncher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(ProjectLauncher PRIVATE 
    IO
    EngineProject
    ${THIRD_PARTY_DIR}/WebView2/WebView2Loader.dll.lib
    user32
    dwmapi
)

add_custom_command(TARGET ProjectLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_DIR}/WebView2/WebView2Loader.dll"
        "$<TARGET_FILE_DIR:ProjectLauncher>/WebView2Loader.dll"
)