include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(ufbx)

# add library
add_library(ufbx STATIC
    ${ufbx_SOURCES}
    ${ufbx_HEADERS}
)

# 指定语言为 C
set_source_files_properties(ufbx PROPERTIES LANGUAGE C)

target_compile_definitions(ufbx PRIVATE UFBX_REAL_IMPLEMENTATION)

target_include_directories(ufbx
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR} # Other projects can access it too
)