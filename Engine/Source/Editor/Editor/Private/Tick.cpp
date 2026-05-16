#include <Editor.h>
#include <EditorRender.h>

#include <IRenderRT.h>

namespace Editor
{
	void Editor::Tick()
	{
		// 这里可以添加每帧需要执行的逻辑，例如处理输入、更新界面等
		EditorRender::Get().Tick();
	}


	void EditorRender::Tick()
	{
		RenderRT::GetRenderRTInterface()->SetRenderTarget();

		editorUI.Tick(); // Tick UI

		// Present
		RenderRT::GetRenderRTInterface()->BindRenderTarget();
		//RenderCore::GetRenderInterface()->DebugD3D11State();
		RenderCore::GetRenderInterface()->Present();
	}
}