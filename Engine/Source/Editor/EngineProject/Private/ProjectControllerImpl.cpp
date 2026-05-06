#include <windows.h>

#include <ProjectControllerImpl.h>
#include <string_view>
#include <Core.h>

///////////////////////////////////
#include <FileManager.h>
#include <Converter.h>
#include <AbsolutePath.h>
///////////////////////////////////
#include <ILog.h>

namespace EngineProject {
	IProjectController* GetProjectControllerInterface()
	{
		return &ProjectControllerImpl::Get();
	}

	void ProjectControllerImpl::Create(ProJectConfig& config)
	{
		std::wstring ProjectPath = IO::Converter::ToWideString(config.path);
		std::wstring ProjectName = IO::Converter::ToWideString(config.name);
		if (IO::FileManager::Exists(ProjectPath)) {
			IO::FileManager::MakeDirectory(ProjectName);

			// enum list
			constexpr auto all_types = magic_enum::enum_values<ProJectDirEnum>();
			for (auto dir : all_types) {
				std::string_view item = magic_enum::enum_name(dir);
				std::wstring witem = ProjectPath + L"\\" + ProjectName + L"\\" + IO::Converter::ToWideString(std::string(item));

				IO::FileManager::MakeDirectory(witem);
			}

			// Create main file
			std::wstring main_file_path = ProjectPath + L"\\" + ProjectName + L"\\" ProjectJSONName;
			JSON mainjson;
			mainjson[magic_enum::enum_name(ProJectJSON::version)] = Core::Core::GetVersion(); // engine version
			mainjson[magic_enum::enum_name(ProJectJSON::projectname)] = config.name;

			std::string Dump = mainjson.dump(4);
			IO::FileManager::MakeFile(main_file_path);
			IO::FileManager::WriteAllText(main_file_path,Dump);
		}
		else {
			LOG_ERROR("The specified directory does not exist !");
		}
	}

	void ProjectControllerImpl::SpecifyProJect(ProJectConfig& config)
	{
		// 设置工作路径
		std::wstring WorkingDir = IO::Converter::ToWideString(config.path) + L"\\";
		WorkingDir += IO::Converter::ToWideString(config.name) + L"\\";
		SetCurrentDirectoryW(WorkingDir.c_str());
	}
}