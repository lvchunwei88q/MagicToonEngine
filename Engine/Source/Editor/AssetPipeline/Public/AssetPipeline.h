#pragma once
#include "Common/ASSETPIPELINE_API.h"

#include <string>

namespace AssetPipeline {
    enum class UIAssetType {
        Folder,
        File,
        Return,
        JSON,
    };

    struct UIAssetPair {
        const char* src;

        UIAssetPair() = default;
        UIAssetPair(const char* src) : src(src) {};
    };

    class ASSETPIPELINE_API UIAsset {
    public:
        virtual const char* GetIcon(UIAssetType type) const = 0;
    };

    ASSETPIPELINE_API UIAsset* GetUIAsset();

    struct FBXImportSettings
    {
        std::string path;
    };

    class ASSETPIPELINE_API FBXAssetImport {
    public:
        virtual void Import(FBXImportSettings settings) = 0;
    };

    ASSETPIPELINE_API FBXAssetImport* GetFBXAssetImport();
}