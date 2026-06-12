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
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal
)

target_sources(MagicEditor PRIVATE "${CONTENT_DIR}/Logo/resources.rc")

target_link_libraries(MagicEditor PRIVATE 
    Core
    Editor
    EngineProject
)

set_target_properties(MagicEditor PROPERTIES
    LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
    LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS"
    LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS"
)