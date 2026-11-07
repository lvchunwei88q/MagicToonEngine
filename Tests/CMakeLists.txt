include(${PROJECT_SOURCE_DIR}/Engine/Cmake/macro.cmake)

# add test
AddEngineTest(ObjectTest
    Core/ObjectTest.cpp
    Common/Test.h
)