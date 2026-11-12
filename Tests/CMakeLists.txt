include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add test Object Test
AddEngineTest(ObjectTest
    Core/ObjectTest.cpp
    Common/TestFramework.h
)

# add test LibClang Test
AddEngineTest(LibClangTest
    LibClang/LibClangTest.cpp
    Common/TestFramework.h
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


# ...