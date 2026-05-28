include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EditorCore)

# 添加库
add_engine_library(EditorCore SHARED
    ${EditorCore_SOURCES}
    ${EditorCore_HEADERS}
)

target_include_directories(EditorCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # 别的项目不可以访问
        ${THIRD_PARTY_DIR} # 使用反射库
        ${CMAKE_SOURCE_DIR}/Engine/Content
)

target_compile_definitions(EditorCore PRIVATE EDITORCORE_EXPORTS)

target_link_libraries(EditorCore PUBLIC Core)
target_link_libraries(EditorCore PRIVATE Logger)