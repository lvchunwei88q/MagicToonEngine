#include "Scene.h"

namespace RuntimeAssets {
	AUTO_REGISTER(Scene)

	bool Scene::Init()
	{
		EngineAssetImpl::Get().LoadAsset(); // 加载引擎资产
		return true;
	}

	void Scene::Uninstall()
	{
	}

}