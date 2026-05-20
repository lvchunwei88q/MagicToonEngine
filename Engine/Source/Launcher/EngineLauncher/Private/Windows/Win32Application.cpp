#include "Main.h"
#include "LauncherRender.h"
#include <IRender.h>

namespace EngineLauncher
{
	void Init()
	{
		// 先初始化设备在初始化UI，确保UI能正确使用设备资源
		RenderCore::GetRenderInterface()->CreateDeviceD3D(g_hwnd);
		LauncherRender::Get().Init();
	}
}