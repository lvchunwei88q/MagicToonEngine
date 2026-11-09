include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add test Object Test
AddEngineTest(ObjectTest
    Core/ObjectTest.cpp
    Common/Test.h
)

# add test LibClang Test
AddEngineTest(LibClangTest
    LibClang/LibClangTest.cpp
    Common/Test.h
)
set(LIBCLANG_ROOT "${PROJECT_SOURCE_DIR}/Engine/ThirdParty/libclang")
set(LIBCLANG_LIBRARY "${LIBCLANG_ROOT}/libclang.lib")
set(LIBCLANG_RUNTIME "${LIBCLANG_ROOT}/libclang.dll")
if(NOT EXISTS "${LIBCLANG_LIBRARY}" OR NOT EXISTS "${LIBCLANG_RUNTIME}")
    message(FATAL_ERROR
        "LibClangTest requires libclang.lib and libclang.dll in ${LIBCLANG_ROOT}")
endif()
target_include_directories(LibClangTest PRIVATE "${LIBCLANG_ROOT}")
target_link_libraries(LibClangTest PRIVATE "${LIBCLANG_LIBRARY}")
add_custom_command(TARGET LibClangTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBCLANG_RUNTIME}"
        "$<TARGET_FILE_DIR:LibClangTest>"
    VERBATIM
)


# ...