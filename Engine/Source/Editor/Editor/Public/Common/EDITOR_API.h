#pragma once
// 导出/导入宏
#ifdef EDITOR_EXPORTS
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API __declspec(dllimport)
#endif
