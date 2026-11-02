include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(LOGGER)

# add library
add_engine_library(Logger SHARED
    ${LOGGER_SOURCES}
    ${LOGGER_HEADERS}
)

target_include_directories(Logger
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
)

target_compile_definitions(Logger PRIVATE LOG_EXPORTS)

target_link_libraries(Logger PRIVATE Core)