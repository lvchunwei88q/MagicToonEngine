#pragma once
#include <IRender.h>

namespace RenderCore
{
	class RenderCore : public IRender
	{
	public:
		virtual bool CreateDeviceD3D(HWND hWnd);
		virtual bool CleanupDeviceD3D();
		virtual void DebugD3D11State();

		virtual void Present();
	};
}