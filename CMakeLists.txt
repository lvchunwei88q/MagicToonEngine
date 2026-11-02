# This is the top-level CMake project file. Global configuration is performed here and subprojects are included.

# We use CMake to build a modular system and classify modules into Public, Private, and Internal categories.

# Special attention must be paid to inter-module dependencies: do not place public interface headers from other modules into this module's Public directory; otherwise, linking errors will occur when the module is referenced externally.

# Unless a module must be accessed by other modules, its interfaces should be placed in the Private directory.

# This approach helps control dependencies between modules and avoids unnecessary coupling.

cmake_minimum_required (VERSION 3.10)
project ("MagicToonEngine")

include(Engine/Cmake/common.cmake)
include(Engine/Cmake/macro.cmake)

# Tools project includes subprojects.
BuildToolsModule(${MAGIC_HEADER_TOOL_TARGET} --generate-dir ${CMAKE_GENERATE_OUTPUT_DIRECTORY})

# Third-party projects.
ThirdPartyModule(IMGUI)
ThirdPartyModule(ufbx)
ThirdPartyModule(SPIRV-Cross)
ThirdPartyModule(WICTextureLoader)
ThirdPartyModule(Lua)

# Engine project includes subprojects.
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

# Set the startup project to Launcher.
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MagicEditor)
# Output path for generated MHT module files.
apply_mht_generate_path()