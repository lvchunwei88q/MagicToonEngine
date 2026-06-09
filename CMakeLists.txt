# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
# 我们使用Cmake 构建模块化系统，并且模块分为 Public，Private(可能还有 Internal)
# 但是我们需要尤其注意对于模块之间的依赖关系，不可以将其他模块的头文件放在 Public 中，
# 除非这个模块确实需要被其他模块访问，否则应该放在 Private 中，这样可以更好地控制模块之间的依赖关系，
# 避免不必要的耦合。
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
ThirdPartyModule(lua)

# Engine 项目 包含子项目。
RuntimeModule(Core)
RuntimeModule(RenderCore)
RuntimeModule(Logger)
RuntimeModule(RenderRT)

EditorModule(EditorCore)
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