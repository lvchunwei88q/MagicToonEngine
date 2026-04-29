#pragma once
#include <Subsystem/SubsystemTemplate.h>
#include <RenderRTInterface.h>

#include <d3d11.h>   
#include <wrl/client.h>  // ComPtr
using Microsoft::WRL::ComPtr;

namespace RenderRT {
	class RenderRT : public RenderRTInterface,
		public SubsystemTemplate<RenderRT,Core::SubsystemContext::Priority::Low> {
	public:

		virtual bool Init();
		virtual void Uninstall();

		virtual void GetGraphicRenderContext(GraphicRenderContext& context);
		virtual void SetGraphicRenderContext(GraphicRenderContext& context);
		virtual void UpdateRenderTargetView(int width, int height);
		virtual void SetRenderTarget();
		virtual void BindRenderTarget();

	private:
		GraphicRenderContext context;

		ComPtr<ID3D11RenderTargetView> MainRenderTarget;
	};
}