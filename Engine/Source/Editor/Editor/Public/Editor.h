#include <Common/EDITOR_API.h>
#include <Common/Singleton.h>

#include <EditorWindows.h>

namespace Editor
{
	class EDITOR_API Editor : public Singleton<Editor>
	{
	public:
		Editor();
		void Run();

	private:
		//EditorWindows::EditorWindows* editorWindows_ = &EditorWindows::EditorWindows::Get();

		void Tick();
	};
}