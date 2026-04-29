include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(LOGGER)

# 添加库
add_engine_library(Logger SHARED
    ${LOGGER_SOURCES}
    ${LOGGER_HEADERS}
)

target_include_directories(Logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(Logger PRIVATE LOG_EXPORTS)

target_link_libraries(Logger PRIVATE Core)

target_link_libraries(Logger PRIVATE IO)