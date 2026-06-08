#pragma once
// 导出/导入宏
#ifdef MBT_EXPORTS
#define MBT_API __declspec(dllexport)
#else
#define MBT_API __declspec(dllimport)
#endif
