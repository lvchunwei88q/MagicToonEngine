# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.10)

include(Engine/Cmake/common.cmake)
include(Engine/Cmake/macro.cmake)
project ("MagicToonEngine")

# 包含子项目。
EngineModule(Core/Core)
EngineModule(Editor/IO)
EngineModule(Editor/Logger)
EngineModule(Launcher)

# 设置启动项目为 Launcher
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Launcher)