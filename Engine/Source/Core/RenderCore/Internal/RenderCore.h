#pragma once
#include <Renderinterface.h>

namespace RenderCore
{
	class RenderCore : public RenderInterface
	{
	public:
		virtual bool CreateDeviceD3D(HWND hWnd);
		virtual bool CleanupDeviceD3D();

		virtual void Present();
	};
}