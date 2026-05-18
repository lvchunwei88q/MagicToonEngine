#pragma once
#include "Common/ENGINEPROJECT_API.h"
#include <string>

namespace EngineProject {

	struct ProJectConfig
	{
		std::string path;
		std::string name;
	};

	class ENGINEPROJECT_API IProjectController // api
	{
	public:
		// 负责项目的创建
		virtual void Create(ProJectConfig& config) = 0;

		// 指定项目 - 引擎当前的项目
		virtual void SpecifyProJect(ProJectConfig& config) = 0;

		// 判断这个项目是否完整
		virtual bool ProjectComplete(ProJectConfig& config) = 0;

		// 加载历史项目
		virtual bool LoadHistoryProjects() = 0;
	};

	ENGINEPROJECT_API IProjectController* GetProjectControllerInterface();
}