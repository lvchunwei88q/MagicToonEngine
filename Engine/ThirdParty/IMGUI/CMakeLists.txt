include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(IMGUI)

add_engine_external_library(IMGUI SHARED
    ${IMGUI_SOURCES}
    ${IMGUI_HEADERS}
)

target_compile_definitions(IMGUI PRIVATE IMGUI_EXPORTS)
target_include_directories(IMGUI
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal
        ${CMAKE_SOURCE_DIR}/Engine/Content
)

target_link_libraries(IMGUI PRIVATE dxgi.lib d3d11.lib d3d12.lib dxguid.lib)

target_link_libraries(IMGUI PRIVATE Logger)
target_link_libraries(IMGUI PRIVATE Core)