include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EngineLauncher)

# 添加库
add_executable(EngineLauncher WIN32 
    ${EngineLauncher_SOURCES}
    ${EngineLauncher_HEADERS}
)

target_include_directories(EngineLauncher
    PRIVATE
        ${THIRD_PARTY_DIR} # 使用WebView2JSON HTTP 库 
        ${CMAKE_SOURCE_DIR}/Engine/Content # 内容目录
)

target_sources(EngineLauncher PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(EngineLauncher PRIVATE 
    IO
    EngineProject
    ${THIRD_PARTY_DIR}/WebView2/WebView2Loader.dll.lib
    user32
    dwmapi
)

add_custom_command(TARGET EngineLauncher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${THIRD_PARTY_DIR}/WebView2/WebView2Loader.dll"
        "$<TARGET_FILE_DIR:EngineLauncher>/WebView2Loader.dll"
)