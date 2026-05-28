include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(MagicEditor)

# 添加库
add_executable(MagicEditor
    ${MagicEditor_SOURCES}
    ${MagicEditor_HEADERS}
)

target_include_directories(MagicEditor
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Engine/Content
)

target_sources(MagicEditor PRIVATE "${CMAKE_SOURCE_DIR}/Engine/Content/Logo/resources.rc")

target_link_libraries(MagicEditor PRIVATE 
    Core
    Editor
    EngineProject
)