include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EngineAsset)

# 添加库
add_engine_library(EngineAsset SHARED
    ${EngineAsset_SOURCES}
    ${EngineAsset_HEADERS}
)

target_include_directories(EngineAsset
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
)

target_compile_definitions(EngineAsset PRIVATE ENGINEASSET_EXPORTS)

target_link_libraries(EngineAsset PRIVATE Core)
target_link_libraries(EngineAsset PRIVATE IO)
target_link_libraries(EngineAsset PRIVATE Logger)

target_link_libraries(EngineAsset PRIVATE ufbx) # 使用FBX 导入器
