#pragma once
// 导出/导入宏
#ifdef RENDERLAUNCHER_EXPORTS
#define RENDERLAUNCHER_API __declspec(dllexport)
#else
#define RENDERLAUNCHER_API __declspec(dllimport)
#endif
