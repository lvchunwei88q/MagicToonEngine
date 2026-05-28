#pragma once
// 导出/导入宏
#ifdef EDITORCORE_EXPORTS
#define EDITORCORE_API __declspec(dllexport)
#else
#define EDITORCORE_API __declspec(dllimport)
#endif
