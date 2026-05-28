#pragma once
// 导出/导入宏
#ifdef ASSETPIPELINE_EXPORTS
#define ASSETPIPELINE_API __declspec(dllexport)
#else
#define ASSETPIPELINE_API __declspec(dllimport)
#endif
