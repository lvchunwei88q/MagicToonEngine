#pragma once

#include <IProjectController.h>
#include <Common/ProjectEnum.h>
#include <ThirdParty.h>

namespace EngineProject {

	class ProjectControllerImpl : public IProjectController, public Singleton<ProjectControllerImpl>
	{
	public:
		virtual void Create(ProJectConfig& config) override;

		// 指定项目 - 引擎当前的项目
		virtual void SpecifyProJect(ProJectConfig& config) override;

		// 判断这个项目是否完整
		virtual bool ProjectComplete(ProJectConfig& config) override;
	private:
	};
}