#pragma once
// 导出/导入宏
#ifdef RENDERUI_EXPORTS
#define RENDERUI_API __declspec(dllexport)
#else
#define RENDERUI_API __declspec(dllimport)
#endif
