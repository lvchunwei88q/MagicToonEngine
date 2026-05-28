#pragma once

#include <Subsystem/SubsystemTemplate.h>

#include "Loaders/UIAssetLoaderImpl.h"
#include "Importers/FBXAssetImportImpl.h"

namespace AssetPipeline {
	/*
	* Asset 存放了所有Editor的数据
	*/

	class Asset final : public SubsystemTemplate<Asset,Core::Priority::Low>
	{
	public:
		virtual bool Init();
		virtual void Uninstall();

	private:
	};
}