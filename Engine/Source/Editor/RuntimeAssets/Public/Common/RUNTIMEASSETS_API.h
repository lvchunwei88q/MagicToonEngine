#pragma once
// 导出/导入宏
#ifdef RUNTIMEASSETS_EXPORTS
#define RUNTIMEASSETS_API __declspec(dllexport)
#else
#define RUNTIMEASSETS_API __declspec(dllimport)
#endif
