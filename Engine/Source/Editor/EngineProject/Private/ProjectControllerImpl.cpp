#include <windows.h>

#include "ProjectControllerImpl.h"
#include <string_view>
#include <Core.h>

///////////////////////////////////
#include <IO.h> 
#include <Tools/SerializeMacro.h>

#include <fstream>
///////////////////////////////////
#include <ILog.h>

namespace EngineProject {
	AUTO_REGISTER(ProjectControllerImpl);

	IProjectController* GetProjectControllerInterface()
	{
		return &ProjectControllerImpl::Get();
	}

	bool ProjectControllerImpl::Init()
	{
		return true;
	}

	void ProjectControllerImpl::Uninstall()
	{
		
	}

	void ProjectControllerImpl::Create(ProJectConfig& config)
	{
		std::wstring ProjectPath = IO::ToWideString(config.path);
		std::wstring ProjectName = IO::ToWideString(config.name);
		if (IO::Exists(ProjectPath)) {
			IO::MakeDirectory(ProjectPath + L"\\" + ProjectName);

			// enum list
			constexpr auto all_types = magic_enum::enum_values<ProJectDirEnum>();
			for (auto dir : all_types) {
				std::string_view item = magic_enum::enum_name(dir);
				std::wstring witem = ProjectPath + L"\\" + ProjectName + L"\\" + IO::ToWideString(std::string(item));

				IO::MakeDirectory(witem);
			}

			// Create main file
			std::wstring main_file_path = ProjectPath + L"\\" + ProjectName + L"\\" ProjectJSONName;
			JSON mainjson;
			mainjson[magic_enum::enum_name(ProJectJSON::version)] = Core::Core::GetVersion(); // engine version
			mainjson[magic_enum::enum_name(ProJectJSON::projectname)] = config.name;

			std::string Dump = mainjson.dump(4);
			IO::MakeFile(main_file_path);
			IO::WriteAllText(main_file_path,Dump);
		}
		else {
			LOG_ERROR("The specified directory does not exist !");
		}
	}

	void ProjectControllerImpl::SpecifyProJect(ProJectConfig& config)
	{
		// 设置工作路径
		std::wstring WorkingDir = IO::ToWideString(config.path) + L"\\";
		WorkingDir += IO::ToWideString(config.name) + L"\\";
		SetCurrentDirectoryW(WorkingDir.c_str());

		// 保留历史
		HProject.existence = true;
		HProject.path = config.path;
		HProject.name = config.name;

		FILE_SERIALIZATION_SAVE(HProject, CONFIG "Project\\", L"HistoryProject.mtdata");
	}

	bool ProjectControllerImpl::ProjectComplete(ProJectConfig& config)
	{
		std::wstring pathw = IO::ToWideString(config.path);
		std::wstring namew = IO::ToWideString(config.name);
		std::wstring root = pathw + L"\\" + namew;

		std::wstring project_config = root + L"\\" ProjectJSONName;

		if (IO::Exists(project_config)) {
			for (auto dir : RequiredDirs) {
				std::string_view name = magic_enum::enum_name(dir);
				std::wstring path = root + L"\\" + IO::ToWideString(std::string(name));

				if (!IO::Exists(path)) {// 必要目录存在
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool ProjectControllerImpl::LoadHistoryProjects()
	{
		FILE_SERIALIZATION_LOADING(HProject, CONFIG "Project\\", L"HistoryProject.mtdata");

		if (HProject.existence) {
			ProJectConfig config;
			config.name = HProject.name;
			config.path = HProject.path;
			SpecifyProJect(config);
			return true;
		}
		return false;
	}
}