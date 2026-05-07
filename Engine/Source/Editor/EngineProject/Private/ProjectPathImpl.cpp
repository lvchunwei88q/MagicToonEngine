#include <ProjectPathImpl.h>
///////////////////////////////////
#include <FileManager.h>
#include <Converter.h>
#include <AbsolutePath.h>
///////////////////////////////////
#include <ILog.h>
#include <Core.h>

#include <string_view>

namespace EngineProject {
	IProjectPath* GetProjectPathInterface()
	{
		return &ProjectPathImpl::Get();
	}

	std::wstring ProjectPathImpl::GetRoot()
	{
		// 一般工作目录就是项目目录
		return IO::AbsolutePath::Get().GetCurrentWorkingDirectory();
	}

	std::wstring ProjectPathImpl::GetSubDir(ProJectDirEnum Enum)
	{
		std::wstring root = IO::AbsolutePath::Get().GetCurrentWorkingDirectory();

		if (!magic_enum::enum_contains(Enum)) {
			LOG_WARNING("No corresponding path!");
			return std::wstring();
		}

		std::string_view name = magic_enum::enum_name(Enum);
		std::wstring wname(name.begin(), name.end());
		return root + L"\\" + wname + L"\\";
	}

	bool ProjectPathImpl::IsVersionCompatible(const std::string& currentVersion,
		const std::string& requiredVersion) {
		// 分割版本号
		auto split = [](const std::string& ver) -> std::vector<int> {
			std::vector<int> parts;
			std::stringstream ss(ver);
			std::string part;
			while (std::getline(ss, part, '.')) {
				parts.push_back(std::stoi(part));
			}
			return parts;
		};

		auto v1 = split(currentVersion);
		auto v2 = split(requiredVersion);

		size_t maxLen = std::max(v1.size(), v2.size());
		v1.resize(maxLen, 0);
		v2.resize(maxLen, 0);

		for (size_t i = 0; i < maxLen; i++) {
			if (v1[i] < v2[i]) return false;  // 当前版本小于要求版本
			if (v1[i] > v2[i]) return true;   // 当前版本大于要求版本
		}
		return true;  // 版本相等
	}

	bool ProjectPathImpl::Detection()
	{
		std::wstring root = IO::AbsolutePath::Get().GetCurrentWorkingDirectory();

		std::wstring project_config = root + L"\\" ProjectJSONName;

		if (IO::FileManager::Exists(project_config)) {
			std::string dump = IO::FileManager::ReadAllText(project_config);
			JSON project = JSON::parse(dump);

			std::string engine_version = Core::Core::GetVersion();
			std::string project_version = project[magic_enum::enum_name(ProJectJSON::version)];

			if (IsVersionCompatible(engine_version, project_version)) {
				// 当前引擎版本 >= x.x.x.x，通过
				std::string_view item_content = magic_enum::enum_name(ProJectDirEnum::Content);
				std::wstring witem_content = root + L"\\" + IO::Converter::ToWideString(std::string(item_content));
				if (IO::FileManager::Exists(witem_content)) { // 必要目录存在
					return true;
				}
			}
		}
		return false;
	}
}