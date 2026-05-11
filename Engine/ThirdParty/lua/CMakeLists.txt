include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(liblua)

# 添加库
add_library(liblua STATIC
    ${liblua_SOURCES}
    ${liblua_HEADERS}
)

target_include_directories(liblua
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # 别的项目也可以访问
)