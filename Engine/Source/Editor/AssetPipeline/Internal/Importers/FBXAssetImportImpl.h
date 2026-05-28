#pragma once

#include "AssetPipeline.h"
#include <Tools/Singleton.h>
#include <Subsystem/SubsystemTemplate.h>

namespace AssetPipeline {
	class FBXAssetImportImpl : public SubsystemTemplate<FBXAssetImportImpl,Core::Priority::Normal> , public FBXAssetImport {
	public:
		virtual bool Init();
		virtual void Uninstall();

		void Import(FBXImportSettings settings);
	};
}