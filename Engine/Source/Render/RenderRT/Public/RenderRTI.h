#pragma once
#include <Common/RENDERRT_API.h>

namespace RenderRT {

	struct GraphicRenderContext {
		int H; int W;
	};

	class RENDERRT_API RenderRTInterface
	{
	public:
		virtual void GetGraphicRenderContext(GraphicRenderContext& context) = 0;
		virtual void SetGraphicRenderContext(GraphicRenderContext& context) = 0;
		virtual void UpdateRenderTargetView(int width, int height) = 0;
		virtual void UpdateBufferManagerViewSize(int width, int height) = 0;
		virtual void SetRenderTarget() = 0;
		virtual void BindRenderTarget() = 0;
	};

	RENDERRT_API RenderRTInterface* GetRenderRTInterface();
}