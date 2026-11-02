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