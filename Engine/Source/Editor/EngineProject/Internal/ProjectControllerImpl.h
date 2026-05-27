#pragma once

#include "IProjectController.h"
#include "Common/ProjectEnum.h"
#include "ThirdParty.h"

#include <Subsystem/SubsystemTemplate.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制
#include <cereal/types/string.hpp>     // std::string

namespace EngineProject {
	struct HistoryProject
	{
		std::string path;
		std::string name;
		bool existence = false;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(path, name, existence);
		}
	};

	class ProjectControllerImpl final : public IProjectController, public SubsystemTemplate<ProjectControllerImpl,Core::Priority::High>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();

		virtual void Create(ProJectConfig& config) override;

		// 指定项目 - 引擎当前的项目
		virtual void SpecifyProJect(ProJectConfig& config) override;

		// 判断这个项目是否完整
		virtual bool ProjectComplete(ProJectConfig& config) override;

		// 加载历史项目
		virtual bool LoadHistoryProjects() override;
	private:
		HistoryProject HProject;
	};
}