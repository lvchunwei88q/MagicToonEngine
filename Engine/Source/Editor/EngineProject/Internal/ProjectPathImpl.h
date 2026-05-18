#pragma once

#include "IProjectPath.h"
#include "ThirdParty.h"

namespace EngineProject {
	class ProjectPathImpl : public IProjectPath,public Singleton<ProjectPathImpl>
	{
	public:
		virtual std::wstring GetRoot() override;
		virtual std::wstring GetSubDir(ProJectDirEnum Enum) override;

		// Detection
		virtual bool Detection() override; // root = CurrentWorkingDirectory

	private:
		bool IsVersionCompatible(const std::string& currentVersion,
			const std::string& requiredVersion);
	};
}