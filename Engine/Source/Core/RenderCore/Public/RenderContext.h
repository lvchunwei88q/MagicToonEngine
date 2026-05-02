#include <Tools/Singleton.h>
#include <d3d11.h>
#include <Common/RENDERCORE_API.h>

namespace RenderCore
{
	// Context save the state of the render system
	class RENDERCORE_API RenderContext : public Singleton<RenderContext>
	{
	public:
		enum class PresentSync : UINT
		{
			Immediate = 0,          // 立即呈现，无垂直同步，可能画面撕裂
			VSync = 1,              // 每1个垂直周期呈现1次 (通常60fps)，平滑无撕裂，推荐
			HalfVSync = 2,          // 每2个垂直周期呈现1次 (通常30fps)，极少使用
		};

		ID3D11Device* g_pd3dDevice = nullptr;
		ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
		IDXGISwapChain* g_pSwapChain = nullptr;

		PresentSync presentSync = PresentSync::VSync; // 默认使用垂直同步
	};
}