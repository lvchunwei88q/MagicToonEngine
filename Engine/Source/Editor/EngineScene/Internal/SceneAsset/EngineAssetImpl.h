#pragma once
#include <SceneAsset/EngineAsset.h>
#include <Tools/Singleton.h>
#include <unordered_map>

namespace EngineScene {
	// 加载 Engine Assets

	class EngineAssetImpl : public EngineAsset, public Singleton<EngineAssetImpl>
	{
	public:
		EngineAssetImpl() {};
		void LoadAsset();

		virtual const char* GetIcon(EngineAssetType type, bool hover) const override;
	private:
		std::unordered_map<EngineAssetType, EngineAssetPair> m_Icons;
	};
}