#pragma once
// 导出/导入宏
#ifdef EDITORWINDOWS_EXPORTS
#define EDITORWINDOWS_API __declspec(dllexport)
#else
#define EDITORWINDOWS_API __declspec(dllimport)
#endif
