#pragma once
// 导出/导入宏
#ifdef ENGINEASSET_EXPORTS
#define ENGINEASSET_API __declspec(dllexport)
#else
#define ENGINEASSET_API __declspec(dllimport)
#endif
