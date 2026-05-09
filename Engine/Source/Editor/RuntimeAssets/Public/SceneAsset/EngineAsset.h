#pragma once
#include <Common/RUNTIMEASSETS_API.h>

namespace RuntimeAssets {

    enum class EngineAssetType {
        Folder,
        File,
        Return,
        JSON,
    };

    struct EngineAssetPair {
        const char* src;

        EngineAssetPair() = default;
        EngineAssetPair(const char* src) : src(src) {};
    };

    class RUNTIMEASSETS_API EngineAsset {
    public:
        virtual const char* GetIcon(EngineAssetType type) const = 0;
    };

    RUNTIMEASSETS_API EngineAsset* GetEngineAsset();
}