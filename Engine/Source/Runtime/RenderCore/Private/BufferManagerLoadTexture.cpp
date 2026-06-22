#include <CoreMinimal.h>
#include "IBufferManager.h"
#include <RenderContext.h>

#include <WICTextureLoader.h>

namespace RenderCore {
    void LoadTextureFromFile(const wchar_t* filePath,const char* textureName)
    {
        ID3D11Device* device = RenderContext::Get().g_pd3dDevice;

        if (device == nullptr) {
            Core::ErrorCapture::Capture("Reading textures is now prohibited because the device has not been created yet.");
            return;
        }

        ID3D11Resource* texture = nullptr;
        ID3D11ShaderResourceView* textureView = nullptr;

        HRESULT hr = DirectX::CreateWICTextureFromFile(
            device,
            filePath,
            &texture,
            &textureView,
            0
        );

        if (FAILED(hr))
        {
            if (texture) texture->Release();
            if (textureView) textureView->Release();
            Core::ErrorCapture::Capture(std::string("Failed to load texture! ") + "Error Code:" + std::to_string(hr));
            return;
        }

        ResourcesBufferContext context;
        context.BufferName = textureName;
        context.Resources = texture;
        context.ResourcesSRV = textureView;

        GetBufferManagerUserInterface()->RegisterResourcesBuffer(context);
    }
}