#include <Editor.h>
#include <Log.h>

namespace EditorWindows
{

	struct AutoRegister
	{
		AutoRegister() {
			Editor::Get().Register<Editor, Core::SubsystemContext::Priority::Normal>();
		}
	};
	static AutoRegister s_AutoRegister;

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