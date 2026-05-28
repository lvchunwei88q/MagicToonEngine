include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(RenderEditor)

# 添加库
add_engine_library(RenderEditor SHARED
    ${RenderEditor_SOURCES}
    ${RenderEditor_HEADERS}
)

target_include_directories(RenderEditor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问
        ${THIRD_PARTY_DIR} # 使用反射库
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(RenderEditor PRIVATE RENDEREDITOR_EXPORTS)


target_link_libraries(RenderEditor PRIVATE RenderUI)
target_link_libraries(RenderEditor PRIVATE RenderCore)
target_link_libraries(RenderEditor PRIVATE Logger)
target_link_libraries(RenderEditor PRIVATE AssetPipeline)
target_link_libraries(RenderEditor PRIVATE EditorCore)

target_link_libraries(RenderEditor PUBLIC RenderRT)
