#include <BufferManagerImpl.h>
#include <BufferManager.inl>
#include <RenderContext.h>

#include <CoreLogCapture/CoreLogCapture.h>

#include <ostream>

namespace RenderCore //RenderCore
{
    AUTO_REGISTER(BufferManagerImpl)

    IBufferManagerUser* GetBufferManagerUserInterface()
    {
        return &BufferManagerImpl::Get();
    }

    IBufferManagerAdmin* GetBufferManagerAdminInterface()
    {
        return &BufferManagerImpl::Get();
    }

    bool BufferManagerImpl::Init()
    {
        ViewContext view;
        view.ScreenSize = XMINT2(0, 0);
        this->view = view; // 初始化
        return true;
    }

    void BufferManagerImpl::Uninstall()
    {
        // 提前释放
        buffers.buffers.clear();
        buffers.textures.clear();
        buffers.resources.clear();
    }

    void BufferManagerImpl::Initialize(ViewContext context)
    {
        this->view = context; // 初始化
    }

    void BufferManagerImpl::RegisterBuffer(BufferContext<D3D11_BUFFER_DESC> context)
    {
        Buffer buffer;
        buffer.Name = context.BufferName;
        buffer.Desc = *context.Desc;

        HRESULT hr = RenderContext::Get().g_pd3dDevice->CreateBuffer(context.Desc, context.InitialData, buffer.buffer.GetAddressOf());

        if (FAILED(hr))
        {
            Core::ErrorCapture::Capture("Error when creating buffer - "
				"Error code:" + std::to_string(hr) + " - Creating buffer name:" + buffer.Name);
            return;
        }

        // 写入时加锁
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            buffers.buffers.push_back(buffer);
        }
    }

    void BufferManagerImpl::RegisterTexture2DBuffer(BufferContext<D3D11_TEXTURE2D_DESC> context)
    {
        IsRegisterTextureBuffer = true;

        TextureBuffer textureBuffer;
        textureBuffer.Name = context.BufferName;
        textureBuffer.ViewSize = XMINT2(context.Desc->Width, context.Desc->Height);
        textureBuffer.FixedSize = context.FixedSize;
        if (!context.FixedSize) // 没有设置固定大小
        {
            textureBuffer.ViewSize = this->view.ScreenSize;
            context.Desc->Width = this->view.ScreenSize.x;
            context.Desc->Height = this->view.ScreenSize.y;
        }

        {
            textureBuffer.Desc = context.Desc != nullptr ? *context.Desc : D3D11_TEXTURE2D_DESC(); // 方便重组（更改分辨率）
            textureBuffer.InitialData = context.InitialData != nullptr ? *context.InitialData : D3D11_SUBRESOURCE_DATA(); // 方便重组（更改分辨率）
            textureBuffer.CreateType = context.TextureCreateType; // 方便重组（更改分辨率）

            textureBuffer.custom_desc = CUSTOM_DESC::NONE;
            if (context.SRVDesc != nullptr)
            {
                textureBuffer.SRVDesc = *context.SRVDesc; // 方便重组（更改分辨率）
                textureBuffer.custom_desc = textureBuffer.custom_desc | CUSTOM_DESC::SRV;
            }
            if (context.DSVDesc != nullptr)
            {
                textureBuffer.DSVDesc = *context.DSVDesc; // 方便重组（更改分辨率）
                textureBuffer.custom_desc = textureBuffer.custom_desc | CUSTOM_DESC::DSV;
            }
            if (context.RTVDesc != nullptr)
            {
                textureBuffer.RTVDesc = *context.RTVDesc; // 方便重组（更改分辨率）
                textureBuffer.custom_desc = textureBuffer.custom_desc | CUSTOM_DESC::RTV;
            }
            if (context.UAVDesc != nullptr)
            {
                textureBuffer.UAVDesc = *context.UAVDesc; // 方便重组（更改分辨率）
                textureBuffer.custom_desc = textureBuffer.custom_desc | CUSTOM_DESC::UAV;
            }
        }

        HRESULT hr = RenderContext::Get().g_pd3dDevice->CreateTexture2D(context.Desc, context.InitialData, textureBuffer.Texture.GetAddressOf());

        if (FAILED(hr))
        {
            Core::ErrorCapture::Capture("Error when creating texture buffer - "
				"Error code:" + std::to_string(hr) + " - Creating buffer name:" + textureBuffer.Name);
            return;
        }

        // TODO Hr的值使用不准确
        if (EnumHasAnyFlags(context.TextureCreateType, TextureCreateType::RTV))
        {
            hr = RenderContext::Get().g_pd3dDevice->CreateRenderTargetView(textureBuffer.Texture.Get(), context.RTVDesc, textureBuffer.RTV.GetAddressOf());
        }
        if (EnumHasAnyFlags(context.TextureCreateType, TextureCreateType::SRV))
        {
            hr = RenderContext::Get().g_pd3dDevice->CreateShaderResourceView(textureBuffer.Texture.Get(), context.SRVDesc, textureBuffer.SRV.GetAddressOf());
        }
        if (EnumHasAnyFlags(context.TextureCreateType, TextureCreateType::DSV))
        {
            hr = RenderContext::Get().g_pd3dDevice->CreateDepthStencilView(textureBuffer.Texture.Get(), context.DSVDesc, textureBuffer.DSV.GetAddressOf());
        }
        if (EnumHasAnyFlags(context.TextureCreateType, TextureCreateType::UAV))
        {
            hr = RenderContext::Get().g_pd3dDevice->CreateUnorderedAccessView(textureBuffer.Texture.Get(), context.UAVDesc, textureBuffer.UAV.GetAddressOf());
        }

        if (FAILED(hr))
        {
            Core::ErrorCapture::Capture("Error when creating RTV/SRV - "
                "Error code:" + std::to_string(hr) + " - Texture name:" + textureBuffer.Name);
            return;
        }


        // 写入时加锁
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            buffers.textures.push_back(textureBuffer);
        }
        //buffers.textures.push_back(textureBuffer);
        IsRegisterTextureBuffer = false;
    }

    void BufferManagerImpl::RegisterResourcesBuffer(ResourcesBufferContext context)
    {
        ID3D11Texture2D* texture2D = nullptr;
        HRESULT hr = context.Resources->QueryInterface(IID_PPV_ARGS(&texture2D));

        if (SUCCEEDED(hr))
        {
            texture2D->Release();
        }
        else {
            Core::ErrorCapture::Capture("Failed to convert to texture!");
            return;
        }

        ResourcesTextureBuffer textureBuffer;
        textureBuffer.Texture = texture2D;
        textureBuffer.Name = context.BufferName;
        textureBuffer.SRV = context.ResourcesSRV;

        if (textureBuffer.Texture == nullptr || textureBuffer.Name.c_str() == nullptr || textureBuffer.SRV == nullptr) {
            Core::ErrorCapture::Capture("Content missing when registering resources!");
            return;
        }

        // 写入时加锁
        {
            std::lock_guard<std::mutex> lock(_Mutex);
            buffers.resources.push_back(textureBuffer);
        }
    }

    void BufferManagerImpl::UpdateBuffers(ViewContext context)
    {
        if (IsRegisterTextureBuffer) {
            Core::ErrorCapture::Capture("Updating the texture buffer is not allowed when registering the texture buffer!");
            return;
        }

        this->view = context;
        // 只需更新buffers中的textures
        for (unsigned int i = 0; i < buffers.textures.size(); i++)
        {
            auto& texture = buffers.textures[i];
            if (texture.FixedSize) continue;

            texture.ViewSize = this->view.ScreenSize;
            texture.Desc.Width = texture.ViewSize.x;
            texture.Desc.Height = texture.ViewSize.y;

            HRESULT hr = 0;
            if (texture.InitialData.pSysMem != nullptr)
                hr = RenderContext::Get().g_pd3dDevice->CreateTexture2D(&texture.Desc, &texture.InitialData, &texture.Texture);
            else
                hr = RenderContext::Get().g_pd3dDevice->CreateTexture2D(&texture.Desc, nullptr, &texture.Texture);
            if (FAILED(hr))
            {
                Core::ErrorCapture::Capture("Error when creating texture buffer - "
					"Error code:" + std::to_string(hr) + " - Creating buffer name:" + texture.Name);
                continue;
            }

            // TODO Hr的值使用不准确
            if (EnumHasAnyFlags(texture.CreateType, TextureCreateType::RTV))
            {
                hr = RenderContext::Get().g_pd3dDevice->CreateRenderTargetView(texture.Texture.Get(),
                    EnumHasAnyFlags(texture.custom_desc, CUSTOM_DESC::RTV) ? &texture.RTVDesc : nullptr, &texture.RTV);
            }
            if (EnumHasAnyFlags(texture.CreateType, TextureCreateType::SRV))
            {
                hr = RenderContext::Get().g_pd3dDevice->CreateShaderResourceView(texture.Texture.Get(),
                    EnumHasAnyFlags(texture.custom_desc, CUSTOM_DESC::SRV) ? &texture.SRVDesc : nullptr, &texture.SRV);
            }
            if (EnumHasAnyFlags(texture.CreateType, TextureCreateType::DSV))
            {
                hr = RenderContext::Get().g_pd3dDevice->CreateDepthStencilView(texture.Texture.Get(),
                    EnumHasAnyFlags(texture.custom_desc, CUSTOM_DESC::DSV) ? &texture.DSVDesc : nullptr, &texture.DSV);
            }
            if (EnumHasAnyFlags(texture.CreateType, TextureCreateType::UAV))
            {
                hr = RenderContext::Get().g_pd3dDevice->CreateUnorderedAccessView(texture.Texture.Get(),
                    EnumHasAnyFlags(texture.custom_desc, CUSTOM_DESC::UAV) ? &texture.UAVDesc : nullptr, &texture.UAV);
            }

            if (FAILED(hr))
            {
                Core::ErrorCapture::Capture("Error when creating RTV/SRV - "
					"Error code:" + std::to_string(hr) + " - Texture name:" + texture.Name);
                continue;
            }

        }
    }

    // --------------------------------------- Get
    ID3D11Buffer* BufferManagerImpl::GetBuffer(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, buffers, buffer, Get)
    }

    ID3D11Texture2D* BufferManagerImpl::GetTexture2D(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, Texture, Get)
    }

    ID3D11ShaderResourceView* BufferManagerImpl::GetTextureSRV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, SRV, Get)
    }

    ID3D11RenderTargetView* BufferManagerImpl::GetTextureRTV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, RTV, Get)
    }

    ID3D11DepthStencilView* BufferManagerImpl::GetTextureDSV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, DSV, Get)
    }

    ID3D11UnorderedAccessView* BufferManagerImpl::GetTextureUAV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, UAV, Get)
    }

    ID3D11Buffer** BufferManagerImpl::GetAddressOfBuffer(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, buffers, buffer, GetAddressOf)
    }

    ID3D11Texture2D** BufferManagerImpl::GetAddressOfTexture2D(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, Texture, GetAddressOf)
    }

    ID3D11ShaderResourceView** BufferManagerImpl::GetAddressOfTextureSRV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, SRV, GetAddressOf)
    }

    ID3D11RenderTargetView** BufferManagerImpl::GetAddressOfTextureRTV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, RTV, GetAddressOf)
    }

    ID3D11DepthStencilView** BufferManagerImpl::GetAddressOfTextureDSV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, DSV, GetAddressOf)
    }

    ID3D11UnorderedAccessView** BufferManagerImpl::GetAddressOfTextureUAV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, textures, UAV, GetAddressOf)
    }

    // resources
    ID3D11Texture2D* BufferManagerImpl::GetRTexture2D(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, resources, Texture, Get)
    }

    ID3D11ShaderResourceView* BufferManagerImpl::GetRTextureSRV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, resources, SRV, Get)
    }
    ID3D11Texture2D** BufferManagerImpl::GetAddressOfRTexture2D(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, resources, Texture, GetAddressOf)
    }
    ID3D11ShaderResourceView** BufferManagerImpl::GetAddressOfRTextureSRV(std::string name)
    {
        GET_BUFFERS_FUNCTION(name, resources, SRV, GetAddressOf)
    }
}// namespace RenderCore