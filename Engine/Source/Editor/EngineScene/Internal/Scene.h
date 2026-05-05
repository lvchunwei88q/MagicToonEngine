#pragma once

#include <Subsystem/SubsystemTemplate.h>

#include <SceneAsset/EngineAssetImpl.h>

namespace EngineScene {
	/*
	* Scene 存放了所有渲染的数据，因为只有在场景的数据才会被渲染，所以Scene代表所有数据，并且Engine只有一个场景
	*/

	class Scene : public SubsystemTemplate<Scene,Core::SubsystemContext::Priority::Low>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();

	private:
	};
}