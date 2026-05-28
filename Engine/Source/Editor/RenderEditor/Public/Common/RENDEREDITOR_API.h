#pragma once
// 导出/导入宏
#ifdef RENDEREDITOR_EXPORTS
#define RENDEREDITOR_API __declspec(dllexport)
#else
#define RENDEREDITOR_API __declspec(dllimport)
#endif
