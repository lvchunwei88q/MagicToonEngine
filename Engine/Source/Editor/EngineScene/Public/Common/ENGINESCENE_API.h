#pragma once
// 导出/导入宏
#ifdef ENGINESCENE_EXPORTS
#define ENGINESCENE_API __declspec(dllexport)
#else
#define ENGINESCENE_API __declspec(dllimport)
#endif
