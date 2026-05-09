#include <Common/RUNTIMEASSETS_API.h>
#include <Tools/Singleton.h>

#include <string>

namespace RuntimeAssets {

	struct FBXImportSettings
	{
		std::string path;
	};

	class RUNTIMEASSETS_API FBXAssetImport : public Singleton<FBXAssetImport> {
	public:
		void Import(FBXImportSettings settings);
	};
}