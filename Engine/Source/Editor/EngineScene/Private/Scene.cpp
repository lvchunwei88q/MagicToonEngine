#include <Scene.h>

namespace EngineScene {
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