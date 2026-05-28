# Scene 存放了所有渲染的数据，因为只有在场景的数据才会被渲染，所以Scene代表所有数据，并且Engine只有一个场景


include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(AssetPipeline)

# 添加库
add_engine_library(AssetPipeline SHARED
    ${AssetPipeline_SOURCES}
    ${AssetPipeline_HEADERS}
)

target_include_directories(AssetPipeline
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(AssetPipeline PRIVATE ASSETPIPELINE_EXPORTS)

target_link_libraries(AssetPipeline PUBLIC Core)
target_link_libraries(AssetPipeline PRIVATE RenderCore)
target_link_libraries(AssetPipeline PRIVATE Logger)

target_link_libraries(AssetPipeline PRIVATE ufbx) # 使用FBX 导入器
