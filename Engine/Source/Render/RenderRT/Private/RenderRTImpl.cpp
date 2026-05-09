#include <RenderRTImpl.h>
#include <RenderContext.h>
#include <IBufferManager.h>
#include <Tools/Debouncer.h>
#include <ILog.h>

namespace RenderRT {
    AUTO_REGISTER(RenderRTImpl)

    IRenderRT* GetRenderRTInterface()
	{
		return &RenderRTImpl::Get();
	}
	bool RenderRTImpl::Init()
	{
		// 创建 RenderTarget
        ComPtr<ID3D11Texture2D> backBuffer;

        HRESULT hr = RenderCore::RenderContext::Get().g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
        if (FAILED(hr)) {
            OutputDebugStringA("GetBuffer failed!\n");
            return false;
        }

        hr = RenderCore::RenderContext::Get().g_pd3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, MainRenderTarget.GetAddressOf());
        if (FAILED(hr)) {
            OutputDebugStringA("CreateRenderTargetView failed!\n");
            return false;
        }

		return true;
	}
	void RenderRTImpl::Uninstall()
	{
	}

    void RenderRTImpl::GetGraphicRenderContext(GraphicRenderContext& context)
    {
        // 返回当前的Imgui渲染窗口大小
        context = this->context;
    }
    void RenderRTImpl::SetGraphicRenderContext(GraphicRenderContext& context)
    {
        this->context = context;
    }
    void RenderRTImpl::UpdateRenderTargetView(int width, int height)
    {
        if (width == 0 || height == 0) return;

        // 释放旧 RTV
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
        MainRenderTarget.Get()->Release();

        // 等待完成
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->ClearState();
        // 调整交换链
        RenderCore::RenderContext::Get().g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        // 重新创建 RTV
        ID3D11Texture2D* pBackBuffer = nullptr;
        RenderCore::RenderContext::Get().g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        RenderCore::RenderContext::Get().g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, MainRenderTarget.GetAddressOf());
        pBackBuffer->Release();

        D3D11_VIEWPORT viewport = {};
        viewport.Width = (float)width;
        viewport.Height = (float)height;
        viewport.MaxDepth = 1.0f;
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->RSSetViewports(1, &viewport);
    }

    void RenderRTImpl::UpdateBufferManagerViewSize(int width, int height)
    {
        static int static_width = 0;
        static int static_height = 0;
        static Debouncer debouncer(500, [&]() {
            LOG_INFO("Update BufferManager ViewSize: " ,static_width , "-" , static_height);
        });

        if (static_width != width || static_height != height) {
            RenderCore::ViewContext context;
            context.ScreenSize.x = width;
            context.ScreenSize.y = height;
            RenderCore::GetBufferManagerAdminInterface()->UpdateBuffers(context);
            debouncer.update(); // 防抖处理

            static_width = width;
            static_height = height;
        }

        debouncer.tick();
    }

    void RenderRTImpl::SetRenderTarget()
    {
        const float clear_color[4] = { 0,0,0,1 };
        ID3D11RenderTargetView* rtv = MainRenderTarget.Get();
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->ClearRenderTargetView(rtv, clear_color);
    }

    void RenderRTImpl::BindRenderTarget()
    {
        ID3D11RenderTargetView* rtv = MainRenderTarget.Get();
        RenderCore::RenderContext::Get().g_pd3dDeviceContext->OMSetRenderTargets(1, &rtv, nullptr);
    }
}