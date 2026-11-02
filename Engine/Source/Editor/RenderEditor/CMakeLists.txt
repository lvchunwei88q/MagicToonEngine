include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderEditor)

# add library
add_engine_library(RenderEditor SHARED
    ${RenderEditor_SOURCES}
    ${RenderEditor_HEADERS}
)

target_include_directories(RenderEditor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR} # 使用反射库
)

target_compile_definitions(RenderEditor PRIVATE RENDEREDITOR_EXPORTS)


target_link_libraries(RenderEditor PRIVATE RenderUI)
target_link_libraries(RenderEditor PRIVATE RenderCore)
target_link_libraries(RenderEditor PRIVATE Logger)
target_link_libraries(RenderEditor PRIVATE AssetPipeline)

target_link_libraries(RenderEditor PUBLIC RenderRT)
