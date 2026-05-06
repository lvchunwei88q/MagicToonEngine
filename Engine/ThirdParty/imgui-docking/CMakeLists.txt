include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(IMGUI)

# 添加库
add_library(IMGUI STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_HEADERS}
)

target_include_directories(IMGUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # 别的项目也可以访问
)

target_link_libraries(IMGUI PRIVATE IO)