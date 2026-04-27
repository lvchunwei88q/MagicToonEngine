#include <Editor.h>
#include <Log.h>

namespace EditorWindows
{

	AUTO_REGISTER_SINGLETON(Editor, Normal)

	Editor::Editor()
	{
		// not
	}

	bool Editor::Init()
	{
		LOG_INFO("Editor subsystem created.");
		return true;
	}
	void Editor::Uninstall()
	{
	}

	void Editor::CreateWindow()
	{
		// 创建窗口的逻辑
	}
}