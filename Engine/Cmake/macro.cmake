macro(EngineModule)
    add_subdirectory("Engine/Source/${ARGV}")
endmacro()

macro(add_module_sources ModuleName)
file(GLOB_RECURSE ${ModuleName}_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.cc"
    "*.cxx"
)

file(GLOB_RECURSE ${ModuleName}_HEADERS 
    CONFIGURE_DEPENDS
    "*.h"
    "*.hpp"
)
endmacro()