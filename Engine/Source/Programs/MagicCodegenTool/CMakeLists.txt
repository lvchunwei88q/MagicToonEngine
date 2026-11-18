include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicCodegenTool)

# add library
add_executable(MagicCodegenTool 
    ${MagicCodegenTool_SOURCES}
    ${MagicCodegenTool_HEADERS}
)

target_include_directories(MagicCodegenTool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR}
)

# add core library
target_link_libraries(MagicCodegenTool PRIVATE Core)

set(LIBCLANG_ROOT "${PROJECT_SOURCE_DIR}/Engine/ThirdParty/libclang")
set(LIBCLANG_LIBRARY "${LIBCLANG_ROOT}/libclang.lib")
set(LIBCLANG_RUNTIME "${LIBCLANG_ROOT}/libclang.dll")
if(NOT EXISTS "${LIBCLANG_LIBRARY}" OR NOT EXISTS "${LIBCLANG_RUNTIME}")
    message(FATAL_ERROR
        "MagicCodegenTool requires libclang.lib and libclang.dll in ${LIBCLANG_ROOT}")
endif()

# Add libclang library
target_include_directories(MagicCodegenTool PRIVATE "${LIBCLANG_ROOT}")
target_link_libraries(MagicCodegenTool PRIVATE "${LIBCLANG_LIBRARY}")
add_custom_command(TARGET MagicCodegenTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBCLANG_RUNTIME}"
        "$<TARGET_FILE_DIR:MagicCodegenTool>"
    VERBATIM
)
