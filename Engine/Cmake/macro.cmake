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
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${ENGINE_COMPILE_COMMANDS_FILE}"
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
macro(AddEngineTest TARGET_NAME)
    if(ARGC LESS 2)
        message(FATAL_ERROR "AddEngineTest(${TARGET_NAME}) requires at least one source file")
    endif()

    add_engine_test_library(${TARGET_NAME} ${ARGN})
    foreach(TEST_FILE ${ARGN})
        get_filename_component(TEST_FILE_ABSOLUTE "${TEST_FILE}" ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
        )
        file(APPEND "${HEADER_LIST_FILE}" "${TEST_FILE_ABSOLUTE} || ${TARGET_NAME}\n")
    endforeach()
    add_dependencies(${TARGET_NAME} ${MAGIC_HEADER_TOOL_RUN})
    
    # Core is the default base library; all arguments after TARGET_NAME are source files.
    target_link_libraries(${TARGET_NAME} PRIVATE Core)
    
    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    # Tests are added from a subdirectory, so register their generated-header path directly.
    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_GENERATE_OUTPUT_DIRECTORY}/Object/${TARGET_NAME}
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
    # Because the core module is fixed, we copied the core module to the test directory
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Core>"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Lua>"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
        VERBATIM
    )
endmacro()

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