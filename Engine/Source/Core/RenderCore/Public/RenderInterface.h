#pragma once
#include <Windows.h>
#include <Common/RENDERCORE_API.h>

namespace RenderCore
{
	class RENDERCORE_API RenderInterface
	{
	public:
		virtual bool CreateDeviceD3D(HWND hWnd) = 0;
		virtual bool CleanupDeviceD3D() = 0;
		virtual void DebugD3D11State() = 0;

		virtual void Present() = 0;
	};

	RENDERCORE_API RenderInterface* GetRenderInterface();
}