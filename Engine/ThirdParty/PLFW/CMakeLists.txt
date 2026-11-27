include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)
include(CMakeDependentOption)

set(PLFW_VERSION_MAJOR 3)
set(PLFW_VERSION_MINOR 2)
set(PLFW_VERSION_PATCH 1)
set(PLFW_VERSION "3.2.1")

cmake_dependent_option(PLFW_BUILD_WIN32 "Build support for Win32" ON "WIN32" OFF)
cmake_dependent_option(PLFW_BUILD_WAYLAND "Build support for Wayland" ON "UNIX" OFF)

# add sources
add_module_sources(PLFW)
add_engine_external_library(PLFW SHARED
    ${PLFW_SOURCES}
    ${PLFW_HEADERS}
)

target_include_directories(PLFW
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Internal
)

target_compile_definitions(PLFW PRIVATE PLFW_EXPORTS)
if (PLFW_BUILD_WIN32)
    target_compile_definitions(PLFW PRIVATE _PLFW_WIN32)
endif()
if (PLFW_BUILD_WAYLAND)
    target_compile_definitions(PLFW PRIVATE _PLFW_WAYLAND)
endif()


# Wayland backend: (1) probe memfd_create() and define HAVE_MEMFD_CREATE
# when available (fallback to shm_open/mkstemp otherwise); (2) locate
# wayland-scanner and fail configuration if it is missing; (3) generate
# client headers and private protocol code from the bundled protocol XML
# files in deps/wayland/ via wayland-scanner, and add the generated
# files to the plfw target sources.
if (PLFW_BUILD_WAYLAND)
    include(CheckFunctionExists)
    check_function_exists(memfd_create HAVE_MEMFD_CREATE)
    if (HAVE_MEMFD_CREATE)
        target_compile_definitions(PLFW PRIVATE HAVE_MEMFD_CREATE)
    endif()

    find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)
    if (NOT WAYLAND_SCANNER_EXECUTABLE)
        message(FATAL_ERROR "Failed to find wayland-scanner")
    endif()

    macro(generate_wayland_protocol protocol_file)
        set(protocol_path "${PLFW_SOURCE_DIR}/deps/wayland/${protocol_file}")

        string(REGEX REPLACE "\\.xml$" "-client-protocol.h" header_file ${protocol_file})
        string(REGEX REPLACE "\\.xml$" "-client-protocol-code.h" code_file ${protocol_file})

        add_custom_command(OUTPUT ${header_file}
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${protocol_path}" ${header_file}
            DEPENDS "${protocol_path}"
            VERBATIM)

        add_custom_command(OUTPUT ${code_file}
            COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${protocol_path}" ${code_file}
            DEPENDS "${protocol_path}"
            VERBATIM)

        target_sources(PLFW PRIVATE ${header_file} ${code_file})
    endmacro()

    generate_wayland_protocol("wayland.xml")
    generate_wayland_protocol("viewporter.xml")
    generate_wayland_protocol("xdg-shell.xml")
    generate_wayland_protocol("idle-inhibit-unstable-v1.xml")
    generate_wayland_protocol("pointer-constraints-unstable-v1.xml")
    generate_wayland_protocol("relative-pointer-unstable-v1.xml")
    generate_wayland_protocol("fractional-scale-v1.xml")
    generate_wayland_protocol("xdg-activation-v1.xml")
    generate_wayland_protocol("xdg-decoration-unstable-v1.xml")
endif()

if (WIN32)
    configure_file(Internal/plfw.rc.in plfw.rc @ONLY)
    target_sources(PLFW PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/plfw.rc")
endif()

if (UNIX)
    # On Unix-like systems, shared libraries can use the soname system.
    set(PLFW_LIB_NAME plfw)
else()
    set(PLFW_LIB_NAME plfw3)
endif()

set_target_properties(PLFW PROPERTIES
                      OUTPUT_NAME ${PLFW_LIB_NAME}
                      VERSION ${PLFW_VERSION_MAJOR}.${PLFW_VERSION_MINOR}
                      SOVERSION ${PLFW_VERSION_MAJOR}
                      POSITION_INDEPENDENT_CODE ON
                      C_STANDARD 99
                      C_EXTENSIONS OFF
                      DEFINE_SYMBOL _PLFW_BUILD_DLL
                      FOLDER "PLFW3")

# Let the target plfw privately link to CMake's Threads::Threads imported target
find_package(Threads REQUIRED)
target_link_libraries(PLFW PRIVATE Threads::Threads)

# When the Wayland backend is enabled, use pkg-config to find the Wayland-related development libraries, require the minimum version, and add their include directories to plfw's private include path.
if (PLFW_BUILD_WAYLAND)
    include(FindPkgConfig)

    pkg_check_modules(Wayland REQUIRED
        wayland-client>=0.2.7
        wayland-cursor>=0.2.7
        xkbcommon>=0.5.0)

    target_include_directories(PLFW PRIVATE ${Wayland_INCLUDE_DIRS})
endif()

# Link two 'non-default' system libraries on Unix for plfw: the math library libm and the dynamic loading library libdl
if (UNIX)
    find_library(MATH_LIBRARY m)
    mark_as_advanced(MATH_LIBRARY)
    if (MATH_LIBRARY)
        target_link_libraries(PLFW PRIVATE "${MATH_LIBRARY}")
    endif()

    if (CMAKE_DL_LIBS)
        target_link_libraries(PLFW PRIVATE "${CMAKE_DL_LIBS}")
    endif()
endif()

# Enable a reasonable set of warnings
# MSVC on Windows, GCC on Linux; no other compiler reaches this file because the
# top-level guard refuses to configure with one
if (MSVC)
    target_compile_options(PLFW PRIVATE "/W3")
else()
    target_compile_options(PLFW PRIVATE "-Wall")
endif()


# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC)
    target_compile_definitions(PLFW PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Workaround for -std=c99 on Linux disabling _DEFAULT_SOURCE (POSIX 2008 and more)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(PLFW PRIVATE _DEFAULT_SOURCE)
endif()
