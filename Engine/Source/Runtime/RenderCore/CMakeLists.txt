include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RENDERCORE)

# add library
add_engine_library(RenderCore SHARED
    ${RENDERCORE_SOURCES}
    ${RENDERCORE_HEADERS}
)

target_include_directories(RenderCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
)

target_compile_definitions(RenderCore PRIVATE RENDERCORE_EXPORTS)

target_link_libraries(RenderCore PUBLIC Core)
target_link_libraries(RenderCore PRIVATE d3d11)
target_link_libraries(RenderCore PRIVATE dxgi)
target_link_libraries(RenderCore PRIVATE WICTextureLoader)