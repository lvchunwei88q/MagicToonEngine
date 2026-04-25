file(GLOB_RECURSE CORE_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.cc"
    "*.cxx"
)

file(GLOB_RECURSE CORE_HEADERS 
    CONFIGURE_DEPENDS
    "*.h"
    "*.hpp"
)

# 添加库
add_library(Core SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public # 别的项目也可以访问

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private # 别的项目不可以访问
)

target_compile_definitions(Core PRIVATE CORE_EXPORTS)