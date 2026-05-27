#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <Tools/EnumClassFlags.h>

#include <Subsystem/SubsystemTemplate.h>

#include <d3d11.h>  // DX 11
#include <wrl/client.h>
using namespace Microsoft::WRL; // ComPtr 的 operator& 会自动 Release 旧对象

//////////////////////////////////////////////////
#include <IBufferManager.h>
//////////////////////////////////////////////////

#define GET_BUFFERS_FUNCTION(target,type,get,getType)\
for (unsigned int i = 0; i < buffers.type.size(); i++)\
{\
    auto& buffer = buffers.type[i];\
    if (buffer.Name == (target))\
    {\
        return buffer.get.getType();\
    }\
}\
return nullptr;

namespace RenderCore //RenderCore
{
    enum class CUSTOM_DESC : uint32_t
    {
        NONE = 0,
        RTV = 1 << 0,  // 1
        SRV = 1 << 1,  // 2
        DSV = 1 << 2,  // 4
        UAV = 1 << 3,  // 8
    };
    ENUM_CLASS_FLAGS(CUSTOM_DESC);

    struct BufferBase // 存放了纹理可以访问的属性
    {
        std::string Name;
    };

    struct TextureBuffer : public BufferBase
    {
        XMINT2 ViewSize;
        bool FixedSize;
        // 组成纹理的基础 RTV and SRV and DSV and UAV
        D3D11_TEXTURE2D_DESC Desc; // desc struct
        D3D11_SUBRESOURCE_DATA InitialData; // data struct
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc; // SRV struct
        D3D11_RENDER_TARGET_VIEW_DESC RTVDesc; // RTV struct
        D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc; // DSV struct
        D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc; // UAV struct
        CUSTOM_DESC custom_desc; // custom desc
        TextureCreateType CreateType; // texture create type

        ComPtr<ID3D11Texture2D> Texture; // default == nullptr
        ComPtr<ID3D11ShaderResourceView> SRV; // default == nullptr
        ComPtr<ID3D11RenderTargetView> RTV; // default == nullptr
        ComPtr<ID3D11DepthStencilView> DSV; // default == nullptr
        ComPtr<ID3D11UnorderedAccessView> UAV; // default == nullptr
    };

    struct ResourcesTextureBuffer : public BufferBase
    {
        XMINT2 ViewSize;
        ComPtr<ID3D11Texture2D> Texture; // default == nullptr
        ComPtr<ID3D11ShaderResourceView> SRV; // default == nullptr
    };

    struct Buffer : public BufferBase
    {
        // 组成缓冲区的基础 Buffer
        D3D11_BUFFER_DESC Desc;
        ComPtr<ID3D11Buffer> buffer; // default == nullptr
    };

    // 存放数据
    struct BufferData
    {
        std::vector<TextureBuffer> textures;
        std::vector<ResourcesTextureBuffer> resources;
        std::vector<Buffer> buffers;
    };

    /**
    * 在注册与更新大小时我们使用线程锁确保在多线程加载时的稳定性，
    * 在获取Texture时我们应该在同一渲染线程中使用这个鞋Get函数来确保不会出现线程问题。
    */
	class BufferManagerImpl final : public IBufferManagerUser, public IBufferManagerAdmin,
        public SubsystemTemplate<BufferManagerImpl,Core::Priority::Core>
    {
    public:

        virtual bool Init();
        virtual void Uninstall();

        // 初始化 一些参数与设置
        virtual void Initialize(ViewContext context) override;
        // 更新各个Buffer大小
        virtual void UpdateBuffers(ViewContext context) override;
        // 注册Buffer
        virtual void RegisterBuffer(BufferContext<D3D11_BUFFER_DESC> context) override;
        // 注册Texture 2D
        virtual void RegisterTexture2DBuffer(BufferContext<D3D11_TEXTURE2D_DESC> context) override;
        // 注册Resources
        virtual void RegisterResourcesBuffer(ResourcesBufferContext context) override;

        // --------------------------------------- Get
        // 获取Buffer
        virtual ID3D11Buffer* GetBuffer(std::string name) override;
        // 获取Texture 2D
        virtual ID3D11Texture2D* GetTexture2D(std::string name) override;
        // 获取 SRV
        virtual ID3D11ShaderResourceView* GetTextureSRV(std::string name) override;
        // 获取 RTV
        virtual ID3D11RenderTargetView* GetTextureRTV(std::string name) override;
        // 获取 DSV
        virtual ID3D11DepthStencilView* GetTextureDSV(std::string name) override;
        // 获取 UAV
        virtual ID3D11UnorderedAccessView* GetTextureUAV(std::string name) override;

        // GetAddressOf Buffer
        virtual ID3D11Buffer** GetAddressOfBuffer(std::string name) override;
        // GetAddressOf Texture 2D
        virtual ID3D11Texture2D** GetAddressOfTexture2D(std::string name) override;
        // GetAddressOf SRV
        virtual ID3D11ShaderResourceView** GetAddressOfTextureSRV(std::string name) override;
        // GetAddressOf RTV
        virtual ID3D11RenderTargetView** GetAddressOfTextureRTV(std::string name) override;
        // GetAddressOf DSV
        virtual ID3D11DepthStencilView** GetAddressOfTextureDSV(std::string name) override;
        // GetAddressOf UAV
        virtual ID3D11UnorderedAccessView** GetAddressOfTextureUAV(std::string name);

        // resources
        // 获取Texture 2D
        virtual ID3D11Texture2D* GetRTexture2D(std::string name) override;
        // 获取 SRV
        virtual ID3D11ShaderResourceView* GetRTextureSRV(std::string name) override;
        // GetAddressOf Texture 2D
        virtual ID3D11Texture2D** GetAddressOfRTexture2D(std::string name) override;
        // GetAddressOf SRV
        virtual ID3D11ShaderResourceView** GetAddressOfRTextureSRV(std::string name) override;
    private:
        std::mutex _Mutex; // 线程锁
        std::atomic<bool> IsRegisterTextureBuffer{ false };

        ViewContext view;
        BufferData buffers;
    };

}// namespace RenderCore