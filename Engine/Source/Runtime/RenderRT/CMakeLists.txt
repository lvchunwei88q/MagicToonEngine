include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderRT)

# add library
add_engine_library(RenderRT SHARED
    ${RenderRT_SOURCES}
    ${RenderRT_HEADERS}
)

target_include_directories(RenderRT
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
)

target_compile_definitions(RenderRT PRIVATE RENDERRT_EXPORTS)

target_link_libraries(RenderRT PRIVATE Core)
target_link_libraries(RenderRT PRIVATE RenderCore)
target_link_libraries(RenderRT PRIVATE Logger)