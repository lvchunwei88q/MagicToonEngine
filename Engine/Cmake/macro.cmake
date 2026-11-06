# ================================================ Engine Modules ================================================
macro(ThirdPartyCoreModule)
    add_subdirectory("Engine/ThirdParty/${ARGV}")
endmacro()

macro(CoreModule)
    add_subdirectory("Engine/Source/Core/${ARGV}")
endmacro()

macro(BuildToolsModule)
    set(ModuleName ${ARGV0})
    set(ModuleArgs "")
    if(${ARGC} GREATER 1)
        set(ModuleArgs ${ARGN})
    endif()

    add_subdirectory("Engine/Source/Programs/${ModuleName}")
    add_custom_target(${MAGIC_HEADER_TOOL_RUN} ALL
        COMMAND $<TARGET_FILE:${ModuleName}> ${ModuleArgs}
        DEPENDS ${ModuleName}
        COMMENT "Running ${ModuleName}..."
    )
endmacro()

macro(ThirdPartyModule)
    add_subdirectory("Engine/ThirdParty/${ARGV}")
endmacro()

macro(RuntimeModule)
    add_subdirectory("Engine/Source/Runtime/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_LINKPATH ${ARGV})
endmacro()

macro(ProgramModule)
    add_subdirectory("Engine/Source/Programs/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_LINKPATH ${ARGV})
endmacro()

macro(EditorModule)
    add_subdirectory("Engine/Source/Editor/${ARGV}")
    add_dependencies(${ARGV} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_LINKPATH ${ARGV})
endmacro()

# ================================================ Cmake Utilities ================================================
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

    # Write the list of source files into a text file, which will be used for generating reflection code in MHT
    foreach(HEADER ${${ModuleName}_HEADERS})
        file(APPEND ${HEADER_LIST_FILE} "${HEADER} || ${ModuleName}\n")
    endforeach()
    foreach(SOURCE ${${ModuleName}_SOURCES})
        file(APPEND ${HEADER_LIST_FILE} "${SOURCE} || ${ModuleName}\n")
    endforeach()
endmacro()

# Prefix for the module
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
function(add_engine_test_library name)
    add_executable(${name} ${ARGN})
    set_target_properties(${name} PROPERTIES 
        PREFIX "MagicTest-"
    )
endfunction()

# ================================================ add Engine tests ===============================================
function(AddEngineTest TARGET_NAME TEST_DIR)
    add_module_sources(${TARGET_NAME})
    add_engine_test_library(${TARGET_NAME} ${${TARGET_NAME}_SOURCES} ${${TARGET_NAME}_HEADERS})
    add_dependencies(${TARGET_NAME} ${MAGIC_HEADER_TOOL_RUN})
    list(APPEND GENERATE_HEADER_LINKPATH ${TARGET_NAME})
    
    # Set Core as the default base libraries, with ARGN receiving other libraries passed in during the call
    target_link_libraries(${TARGET_NAME} PRIVATE Core ${ARGN})
    
    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Get the base output directory for the target. If the target does not have a specific output directory set, it will inherit the global one.
    get_target_property(BASE_OUTPUT_DIR ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    if(NOT BASE_OUTPUT_DIR)
        set(BASE_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()
    
    # apply the output directory settings for the test target
    set_target_properties(${TARGET_NAME} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY "${BASE_OUTPUT_DIR}/Test"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BASE_OUTPUT_DIR}/Test"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BASE_OUTPUT_DIR}/Test"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${BASE_OUTPUT_DIR}/Test"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${BASE_OUTPUT_DIR}/Test"
    )
endfunction()

# =============================================== MHT tool ===============================================
function(apply_mht_generate_path)
    foreach(module ${GENERATE_HEADER_LINKPATH})
        message(STATUS " This module will use the MHT tool: ${module}")
        # Set the include path for the generated files of this module to the MHT output directory
        target_include_directories(${module} PRIVATE
            ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/Object/${module}
        )
    endforeach()
endfunction()