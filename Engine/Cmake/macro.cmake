macro(BuildToolsModule)
    set(ModuleName ${ARGV0})
    set(ModuleArgs "")
    if(${ARGC} GREATER 1)
        set(ModuleArgs ${ARGN})
    endif()

    add_subdirectory("Engine/Source/Programs/${ModuleName}")
    add_custom_target(Run${ModuleName} ALL
        COMMAND $<TARGET_FILE:${ModuleName}> ${ModuleArgs}
        DEPENDS ${ModuleName}
        COMMENT "Running ${ModuleName}..."
    )
endmacro()

macro(ProgramModule)
    add_subdirectory("Engine/Source/Programs/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_MODEL ${ARGV})
endmacro()

macro(EditorModule)
    add_subdirectory("Engine/Source/Editor/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_MODEL ${ARGV})
endmacro()
macro(RuntimeModule)
    add_subdirectory("Engine/Source/Runtime/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_MODEL ${ARGV})
endmacro()

macro(ThirdPartyModule)
    add_subdirectory("Engine/ThirdParty/${ARGV}")
endmacro()

macro(add_module_sources ModuleName)
file(GLOB_RECURSE ${ModuleName}_SOURCES 
    CONFIGURE_DEPENDS
    "*.cpp"
    "*.cc"
    "*.cxx"
    "*.c"
)

file(GLOB_RECURSE ${ModuleName}_HEADERS 
    CONFIGURE_DEPENDS
    "*.h"
    "*.hpp"
)

# 将源文件列表写入到一个文本文件中
foreach(HEADER ${${ModuleName}_HEADERS})
    file(APPEND ${HEADER_LIST_FILE} "${HEADER} || ${ModuleName}\n")
endforeach()

endmacro()

# 给模块的前缀
function(add_engine_library name)
    add_library(${name} ${ARGN})
    set_target_properties(${name} PROPERTIES 
        PREFIX "Magic-"
    )
endfunction()
function(add_engine_external_library name)
    add_library(${name} ${ARGN})
    set_target_properties(${name} PROPERTIES 
        PREFIX "Magic-External-"
    )
endfunction()

function(apply_mht_generate_path)
    foreach(module ${GENERATE_HEADER_MODEL})
        message(STATUS " This module will use the MHT tool: ${module}")
        target_include_directories(${module} PRIVATE
            ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/${module}
        )
    endforeach()
endfunction()