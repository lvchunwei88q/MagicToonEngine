include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicEditor)

# 添加库
add_executable(MagicEditor
    ${MagicEditor_SOURCES}
    ${MagicEditor_HEADERS}
)

target_include_directories(MagicEditor
    PRIVATE
        ${CONTENT_DIR} # 使用内容目录
)

target_sources(MagicEditor PRIVATE "${CONTENT_DIR}/Logo/resources.rc")

target_link_libraries(MagicEditor PRIVATE 
    Core
    Editor
    EngineProject
)