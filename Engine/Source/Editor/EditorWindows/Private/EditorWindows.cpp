#include <EditorWindows.h>
#include <WindowsConfig.h>
#include <Log.h>

namespace EditorWindows
{

	AUTO_REGISTER_SINGLETON(EditorWindows, Normal)

	EditorWindows::EditorWindows()
	{
		// not
	}

	bool EditorWindows::Init()
	{
		LOG_INFO("Editor subsystem created.");

		// Initialize the editor subsystem, e.g. create windows, initialize resources, etc.
		LOG_INFO("Editor Create Windows.");
		CreateWindows();
		LOG_INFO("Editor Create DeviceD3D.");
		CreateDeviceD3D();

		return true;
	}

	void EditorWindows::Uninstall()
	{
		CleanupWindows();
	}
}