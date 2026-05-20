#include "LauncherRender.h"
#include "Main.h"
#include <IRenderRT.h>

namespace EngineLauncher
{
	void Tick()
	{
		// 这里可以添加每帧需要执行的逻辑，例如处理输入、更新界面等
		LauncherRender::Get().Tick();
	}

	void LauncherRender::Init()
	{
	}

	void LauncherRender::End()
	{
	}

	void LauncherRender::Tick()
	{
		RenderRT::GetRenderRTInterface()->SetRenderTarget();

		// Present
		RenderRT::GetRenderRTInterface()->BindRenderTarget();
		//RenderCore::GetRenderInterface()->DebugD3D11State();
		RenderCore::GetRenderInterface()->Present();
	}
}