#pragma once
// 导出/导入宏
#ifdef RENDERRT_EXPORTS
#define RENDERRT_API __declspec(dllexport)
#else
#define RENDERRT_API __declspec(dllimport)
#endif
