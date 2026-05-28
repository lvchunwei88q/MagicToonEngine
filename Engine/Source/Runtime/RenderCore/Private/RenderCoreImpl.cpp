#include <IRender.h>
#include <RenderCoreImpl.h>
#include <RenderContext.h>
#include <memory>

namespace RenderCore
{

    IRender* GetRenderInterface()
    {
        static RenderCoreImpl instance;
        return &instance;
    }

    // Helper functions   src = "https://github.com/ocornut/imgui/tree/docking"
    bool RenderCoreImpl::CreateDeviceD3D(HWND hWnd)
    {
        // Setup swap chain
        // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
            &RenderContext::Get().g_pSwapChain, &RenderContext::Get().g_pd3dDevice, &featureLevel, &RenderContext::Get().g_pd3dDeviceContext);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, 
                &RenderContext::Get().g_pSwapChain, &RenderContext::Get().g_pd3dDevice, &featureLevel, &RenderContext::Get().g_pd3dDeviceContext);
        if (res != S_OK)
            return false;

        // Disable DXGI's default Alt+Enter fullscreen behavior.
        // - You are free to leave this enabled, but it will not work properly with multiple viewports.
        // - This must be done for all windows associated to the device. Our DX11 backend does this automatically for secondary viewports that it creates.
        IDXGIFactory* pSwapChainFactory;
        if (SUCCEEDED(RenderContext::Get().g_pSwapChain->GetParent(IID_PPV_ARGS(&pSwapChainFactory))))
        {
            pSwapChainFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
            pSwapChainFactory->Release();
        }

#ifdef _DEBUG
        // 获取 Debug 接口
        ID3D11Debug* debugInterface = nullptr;
        RenderContext::Get().g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugInterface);

        if (debugInterface)
        {
            // 可选：设置报告级别
            ID3D11InfoQueue* infoQueue = nullptr;
            debugInterface->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&infoQueue);

            if (infoQueue)
            {
                // 设置哪些类型的消息需要中断（Break）
                infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
                //infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

                infoQueue->Release();
            }
            debugInterface->Release();
        }
#endif
        return true;
    }

    void RenderCoreImpl::DebugD3D11State() {
        auto* ctx = RenderContext::Get().g_pd3dDeviceContext;

        // 查询当前绑定的 RenderTarget
        ID3D11RenderTargetView* rtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
        ID3D11DepthStencilView* dsv = nullptr;
        ctx->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, rtvs, &dsv);

        // 查询 Viewport
        UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
        ctx->RSGetViewports(&numViewports, viewports);

        // 查询绑定的 Shader
        ID3D11VertexShader* vs = nullptr;
        ID3D11PixelShader* ps = nullptr;
        ctx->VSGetShader(&vs, nullptr, nullptr);
        ctx->PSGetShader(&ps, nullptr, nullptr);

        char buf[512];
        sprintf_s(buf,
            "=== DX11 State ===\n"
            "RTV[0]: %p\n"
            "DSV:    %p\n"
            "VP[0]:  %.0f,%.0f %.0fx%.0f\n"
            "VS:     %p\n"
            "PS:     %p\n"
            "==================\n",
            rtvs[0], dsv,
            viewports[0].TopLeftX, viewports[0].TopLeftY,
            viewports[0].Width, viewports[0].Height,
            vs, ps);
        OutputDebugStringA(buf);

        // 释放临时接口
        for (auto& rtv : rtvs) if (rtv) rtv->Release();
        if (dsv) dsv->Release();
        if (vs) vs->Release();
        if (ps) ps->Release();
    }

    bool RenderCoreImpl::CleanupDeviceD3D()
    {
        // 先切换到窗口模式
        if (RenderContext::Get().g_pSwapChain)
        {
            RenderContext::Get().g_pSwapChain->SetFullscreenState(FALSE, nullptr);
        }

        // 清除设备上下文状态
        if (RenderContext::Get().g_pd3dDeviceContext)
        {
            RenderContext::Get().g_pd3dDeviceContext->ClearState();
            RenderContext::Get().g_pd3dDeviceContext->Flush();
        }

        // 释放交换链（必须在设备上下文之前）
        if (RenderContext::Get().g_pSwapChain)
        {
            RenderContext::Get().g_pSwapChain->Release();
            RenderContext::Get().g_pSwapChain = nullptr;
        }

        // 释放设备上下文
        if (RenderContext::Get().g_pd3dDeviceContext)
        {
            RenderContext::Get().g_pd3dDeviceContext->Release();
            RenderContext::Get().g_pd3dDeviceContext = nullptr;
        }

        // 最后释放设备
        if (RenderContext::Get().g_pd3dDevice)
        {
            RenderContext::Get().g_pd3dDevice->Release();
            RenderContext::Get().g_pd3dDevice = nullptr;
        }

        return true;
    }

    void RenderCoreImpl::Present()
    {
        RenderContext::Get().g_pSwapChain->Present((UINT)RenderContext::Get().presentSync, 0);
    }
}