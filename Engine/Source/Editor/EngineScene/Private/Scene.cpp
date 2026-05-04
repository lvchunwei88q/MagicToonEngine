#include <Scene.h>

namespace EngineScene {
	AUTO_REGISTER(Scene)

	bool Scene::Init()
	{
		engineasset.LoadAsset(); // 加载引擎资产
		return true;
	}

	void Scene::Uninstall()
	{
	}

}