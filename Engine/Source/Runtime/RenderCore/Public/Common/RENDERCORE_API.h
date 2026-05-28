#pragma once
// 导出/导入宏
#ifdef RENDERCORE_EXPORTS
#define RENDERCORE_API __declspec(dllexport)
#else
#define RENDERCORE_API __declspec(dllimport)
#endif
