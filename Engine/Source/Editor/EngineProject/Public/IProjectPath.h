#pragma once

#include <Common/ENGINEPROJECT_API.h>
#include <Common/ProjectEnum.h>
#include <string>

namespace EngineProject {
	class ENGINEPROJECT_API IProjectPath // api
	{
	public:
		virtual std::wstring GetRoot() = 0;
		virtual std::wstring GetSubDir(ProJectDirEnum Enum) = 0;

		// Detection
		virtual bool Detection() = 0; // root = CurrentWorkingDirectory
	};

	ENGINEPROJECT_API IProjectPath* GetProjectPathInterface();
}