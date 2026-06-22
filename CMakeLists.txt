#CMakeList.txt 是顶层 CMake 项目文件，在此处执行全局配置并包含各子项目。
#CMakeList.txt is the top-level CMake project file, where global configuration is performed and subprojects are included.

#我们使用 CMake 构建模块化系统，并且将模块划分为 Public、Private 和 Internal 三类。
#We use CMake to build a modular system, and modules are classified into Public, Private, and Internal categories.

#需要特别注意的是，对于模块之间的依赖关系，绝不能将其他模块的公开接口头文件放在本模块的 Public 目录中，否则当该模块被外部引用时会导致链接错误。
#Special attention must be paid to inter-module dependencies: you must never place public interface headers from other modules into the Public directory of your own module, otherwise linking errors will occur when that module is referenced externally.

#除非该模块确实需要被其他模块访问，否则相关接口应放在 Private 目录中。
#Unless that module truly needs to be accessed by other modules, the relevant interfaces should be placed in the Private directory.

#这样做可以更好地控制模块之间的依赖关系，避免不必要的耦合。
#This approach allows better control over dependencies between modules and avoids unnecessary coupling.

cmake_minimum_required (VERSION 3.10)
project ("MagicToonEngine")

include(Engine/Cmake/common.cmake)
include(Engine/Cmake/macro.cmake)

# Tools 项目 包含子项目。
BuildToolsModule(${MAGIC_HEADER_TOOL_TARGET} --generate-dir ${CMAKE_GENERATE_OUTPUT_DIRECTORY})

# 第三方项目
ThirdPartyModule(imgui-docking)
ThirdPartyModule(ufbx)
ThirdPartyModule(WICTextureLoader)
ThirdPartyModule(Lua)

# Engine 项目 包含子项目。
RuntimeModule(Core)
RuntimeModule(RenderCore)
RuntimeModule(Logger)
RuntimeModule(RenderRT)

EditorModule(AssetPipeline)
EditorModule(RenderUI)
EditorModule(RenderEditor)
EditorModule(RenderLauncher)
EditorModule(EngineProject)
EditorModule(Editor)
EditorModule(MagicEditor)
EditorModule(MagicLauncher)

# 设置启动项目为 Launcher
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MagicEditor)
# 给模块MHT生成文件的路径
apply_mht_generate_path()