file(GLOB_RECURSE LAUNCHER_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.cc"
    "*.cxx"
)

file(GLOB_RECURSE LAUNCHER_HEADERS 
    CONFIGURE_DEPENDS
    "*.h"
    "*.hpp"
)

# 添加库
add_executable(Launcher
    ${LAUNCHER_SOURCES}
    ${LAUNCHER_HEADERS}
)

target_link_libraries(Launcher PRIVATE Core)