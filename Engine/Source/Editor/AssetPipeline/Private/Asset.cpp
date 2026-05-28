#include "Asset.h"

namespace AssetPipeline {
	AUTO_REGISTER(Asset)

	bool Asset::Init()
	{
		UIAssetImpl::Get().LoadAsset(); // 加载引擎资产
		return true;
	}

	void Asset::Uninstall()
	{
	}

}