include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicHeaderTool)

# 添加库
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
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR}
)