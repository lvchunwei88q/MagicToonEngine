#pragma once
#include <Common/ENGINESCENE_API.h>

namespace EngineScene {

    enum class EngineAssetType {
        Folder,
        File,
        Return,
    };

    struct EngineAssetPair {
        const char* hover;
        const char* noHover;

        EngineAssetPair() = default;
        EngineAssetPair(const char* hover, const char* noHover) : hover(hover), noHover(noHover) {};
    };

    class ENGINESCENE_API EngineAsset {
    public:
        virtual const char* GetIcon(EngineAssetType type, bool hover) const = 0;
    };

    ENGINESCENE_API EngineAsset* GetEngineAsset();
}