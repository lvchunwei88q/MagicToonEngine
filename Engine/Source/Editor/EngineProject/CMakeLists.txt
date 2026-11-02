# Scene 存放了所有渲染的数据，因为只有在场景的数据才会被渲染，所以Scene代表所有数据，并且Engine只有一个场景


include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

add_module_sources(EngineProject)

# add library
add_engine_library(EngineProject SHARED
    ${EngineProject_SOURCES}
    ${EngineProject_HEADERS}
)

target_include_directories(EngineProject
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # Other projects can access it too

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal # Other projects can't be accessed
        ${THIRD_PARTY_DIR} # 使用枚举库
)

target_compile_definitions(EngineProject PRIVATE ENGINEPROJECT_EXPORTS)

target_link_libraries(EngineProject PRIVATE Core)
target_link_libraries(EngineProject PRIVATE Logger)
