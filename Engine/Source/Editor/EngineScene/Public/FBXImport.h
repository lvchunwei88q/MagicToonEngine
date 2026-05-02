#include <Common/ENGINESCENE_API.h>
#include <Tools/Singleton.h>

#include <string>

namespace EngineScene {

	struct FBXImportSettings
	{
		std::string path;
	};

	class ENGINESCENE_API FBXAssetImport : public Singleton<FBXAssetImport> {
	public:
		void Import(FBXImportSettings settings);
	};
}