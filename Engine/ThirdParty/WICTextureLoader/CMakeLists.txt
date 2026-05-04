include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(WICTextureLoader)

# 添加库
add_library(WICTextureLoader STATIC
    ${WICTextureLoader_SOURCES}
    ${WICTextureLoader_HEADERS}
)

target_include_directories(WICTextureLoader
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # 别的项目也可以访问
)