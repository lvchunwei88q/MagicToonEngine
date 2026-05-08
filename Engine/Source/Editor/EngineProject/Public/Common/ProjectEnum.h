#pragma once
#define ProjectJSONName L"engine.config.json"

namespace EngineProject {
	enum class ProJectDirEnum {
		Content,
		Cache,
		Config,
	};

	// 必要目录
	inline constexpr ProJectDirEnum RequiredDirs[] = {
		ProJectDirEnum::Content,
		ProJectDirEnum::Config,
	};

	enum class ProJectJSON
	{
		version,
		projectname,
	};
}