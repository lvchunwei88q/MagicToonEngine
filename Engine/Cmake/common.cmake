# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

if(MSVC)
    # 关闭警告 C4819: 无效的字符。避免中文注释被警告
    add_compile_options(
        "/wd4819"
    )
endif()

# 定义第三方库的根目录
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/Engine/Thirdparty)

# 设置输出目录为项目根目录下的 out/Binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/Binary)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编码 使用 Unicode 编码 UTF-16 Unicode
add_definitions(-DUNICODE)