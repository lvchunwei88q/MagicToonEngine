include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicHeaderTool)

# add library
add_executable(MagicHeaderTool 
    ${MagicHeaderTool_SOURCES}
    ${MagicHeaderTool_HEADERS}
)

if(OFF)
    target_compile_options(MagicHeaderTool PRIVATE
        "$<$<CONFIG:Debug>:/fsanitize=address>"
        "$<$<CONFIG:Debug>:/Zi>"
    )
    target_link_options(MagicHeaderTool PRIVATE
        "$<$<CONFIG:Debug>:/fsanitize=address>"
        "$<$<CONFIG:Debug>:/INCREMENTAL:NO>"
    )
endif()

target_include_directories(MagicHeaderTool
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR}
)

# add core library
target_link_libraries(MagicHeaderTool PRIVATE Core)

set(LIBCLANG_ROOT "${PROJECT_SOURCE_DIR}/Engine/ThirdParty/libclang")
set(LIBCLANG_LIBRARY "${LIBCLANG_ROOT}/libclang.lib")
set(LIBCLANG_RUNTIME "${LIBCLANG_ROOT}/libclang.dll")
if(NOT EXISTS "${LIBCLANG_LIBRARY}" OR NOT EXISTS "${LIBCLANG_RUNTIME}")
    message(FATAL_ERROR
        "MagicHeaderTool requires libclang.lib and libclang.dll in ${LIBCLANG_ROOT}")
endif()

# Add libclang library
target_include_directories(MagicHeaderTool PRIVATE "${LIBCLANG_ROOT}")
target_link_libraries(MagicHeaderTool PRIVATE "${LIBCLANG_LIBRARY}")
add_custom_command(TARGET MagicHeaderTool POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBCLANG_RUNTIME}"
        "$<TARGET_FILE_DIR:MagicHeaderTool>"
    VERBATIM
)
