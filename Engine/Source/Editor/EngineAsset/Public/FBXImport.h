#include <Common/ENGINEASSET_API.h>
#include <Tools/Singleton.h>

#include <string>

namespace EngineAsset {

	struct FBXImportSettings
	{
		std::string path;
	};

	class ENGINEASSET_API FBXAssetImport : public Singleton<FBXAssetImport> {
	public:
		void Import(FBXImportSettings settings);
	};
}