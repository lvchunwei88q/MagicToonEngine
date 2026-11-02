include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicLauncher)

# add library
add_executable(MagicLauncher WIN32 
    ${MagicLauncher_SOURCES}
    ${MagicLauncher_HEADERS}
)

target_include_directories(MagicLauncher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR} # 使用WebView2JSON HTTP 库 
        ${CONTENT_DIR} # 内容目录
)

target_sources(MagicLauncher PRIVATE "${CONTENT_DIR}/Logo/resources.rc")

target_link_libraries(MagicLauncher PRIVATE # windows
    user32
    dwmapi
)

target_link_libraries(MagicLauncher PRIVATE Core)
target_link_libraries(MagicLauncher PRIVATE EngineProject)
target_link_libraries(MagicLauncher PRIVATE RenderCore)
target_link_libraries(MagicLauncher PRIVATE RenderLauncher)