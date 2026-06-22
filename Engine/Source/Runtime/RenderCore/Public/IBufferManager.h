#pragma once
#include "Common/RENDERCORE_API.h"
#include <Tools/EnumClassFlags.h>
// Magic 数学库
#include <CoreMinimal.h>

#include <string>
#include <d3d11.h>

/*
* 接口分为了用户模式与管理员模式，分别对BM进行不同的操控
*/

namespace RenderCore // RenderCore
{
    // 这里控制 Texture Buffer 的创建销毁 并且在窗口变化时更新
    struct ViewContext
    {
        int2 ScreenSize;
    };

    enum class TextureCreateType : uint32_t
    {
        NONE = 0,
        RTV = 1 << 0,// 创建 RTV
        SRV = 1 << 1,// 创建 SRV
        UAV = 1 << 2,// 创建 UAV
        DSV = 1 << 3,// 创建 DSV
    };
    ENUM_CLASS_FLAGS(TextureCreateType);

    template <typename D3D11_DESC>
    struct BufferContext
    {
        D3D11_DESC* Desc = nullptr; // texture or asset
        bool FixedSize = false; // fixed size;
        D3D11_SUBRESOURCE_DATA* InitialData = nullptr;
        D3D11_SHADER_RESOURCE_VIEW_DESC* SRVDesc = nullptr; // SRV struct
        D3D11_RENDER_TARGET_VIEW_DESC* RTVDesc = nullptr; // RTV struct
        D3D11_DEPTH_STENCIL_VIEW_DESC* DSVDesc = nullptr; // DSV struct
        D3D11_UNORDERED_ACCESS_VIEW_DESC* UAVDesc = nullptr; // UAV struct
        TextureCreateType TextureCreateType = TextureCreateType::RTV; // RTV or RTVSRV or NOT
        std::string BufferName = "mytexture";
    };

    struct ResourcesBufferContext {
        ID3D11Resource* Resources = nullptr;
        ID3D11ShaderResourceView* ResourcesSRV = nullptr;
        std::string BufferName = "mytexture";
    };

    class RENDERCORE_API IBufferManagerUser
    {
    public:
        virtual ~IBufferManagerUser() = default;

        // 获取Buffer
        virtual ID3D11Buffer* GetBuffer(std::string name) = 0;
        // 获取Texture 2D
        virtual ID3D11Texture2D* GetTexture2D(std::string name) = 0;
        // 获取 SRV
        virtual ID3D11ShaderResourceView* GetTextureSRV(std::string name) = 0;
        // 获取 RTV
        virtual ID3D11RenderTargetView* GetTextureRTV(std::string name) = 0;
        // 获取 DSV
        virtual ID3D11DepthStencilView* GetTextureDSV(std::string name) = 0;
        // 获取 UAV
        virtual ID3D11UnorderedAccessView* GetTextureUAV(std::string name) = 0;
        // GetAddressOf Buffer
        virtual ID3D11Buffer** GetAddressOfBuffer(std::string name) = 0;
        // GetAddressOf Texture 2D
        virtual ID3D11Texture2D** GetAddressOfTexture2D(std::string name) = 0;
        // GetAddressOf SRV
        virtual ID3D11ShaderResourceView** GetAddressOfTextureSRV(std::string name) = 0;
        // GetAddressOf RTV
        virtual ID3D11RenderTargetView** GetAddressOfTextureRTV(std::string name) = 0;
        // GetAddressOf DSV
        virtual ID3D11DepthStencilView** GetAddressOfTextureDSV(std::string name) = 0;
        // GetAddressOf UAV
        virtual ID3D11UnorderedAccessView** GetAddressOfTextureUAV(std::string name) = 0;
        // resources
        // 获取Texture 2D
        virtual ID3D11Texture2D* GetRTexture2D(std::string name) = 0;
        // 获取 SRV
        virtual ID3D11ShaderResourceView* GetRTextureSRV(std::string name) = 0;
        // GetAddressOf Texture 2D
        virtual ID3D11Texture2D** GetAddressOfRTexture2D(std::string name) = 0;
        // GetAddressOf SRV
        virtual ID3D11ShaderResourceView** GetAddressOfRTextureSRV(std::string name) = 0;

        // 注册Buffer
        virtual void RegisterBuffer(BufferContext<D3D11_BUFFER_DESC> context) = 0;
        // 注册Texture 2D
        virtual void RegisterTexture2DBuffer(BufferContext<D3D11_TEXTURE2D_DESC> context) = 0;
        // 注册Resources
        virtual void RegisterResourcesBuffer(ResourcesBufferContext context) = 0;
    };

    RENDERCORE_API IBufferManagerUser* GetBufferManagerUserInterface();

    class RENDERCORE_API IBufferManagerAdmin
    {
    public:
        virtual ~IBufferManagerAdmin() = default;

        // 初始化 一些参数与设置
        virtual void Initialize(ViewContext context) = 0;
        // 更新各个Buffer大小
        virtual void UpdateBuffers(ViewContext context) = 0;
    };

    RENDERCORE_API IBufferManagerAdmin* GetBufferManagerAdminInterface();

    // ------------------------------------------ Tools Function ---------------------------------------------- //
    void RENDERCORE_API LoadTextureFromFile(const wchar_t* filePath, const char* textureName);
} // namespace RenderCore