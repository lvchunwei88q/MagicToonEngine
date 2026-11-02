include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(WICTextureLoader)

# add library
add_library(WICTextureLoader STATIC
    ${WICTextureLoader_SOURCES}
    ${WICTextureLoader_HEADERS}
)

target_include_directories(WICTextureLoader
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Other projects can access it too
)