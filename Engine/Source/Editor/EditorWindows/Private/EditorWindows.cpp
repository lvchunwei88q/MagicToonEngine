#include <EditorWindows.h>
#include <WindowsConfig.h>
#include <Log.h>

#include <BufferManagerInterface.h> // 初始化管理器

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

		// init buffer Manager
		LOG_INFO("Init Buffer Manager.");
		RenderCore::BufferManagerAdminInterface* BMAdmin = RenderCore::GetBufferManagerAdminInterface();
		WindowsConfig& config = WindowsConfig::Get();
		RenderCore::ViewContext context;
		context.ScreenSize = XMINT2(config.width, config.height);
		BMAdmin->Initialize(context);

		return true;
	}

	void EditorWindows::Uninstall()
	{
		CleanupWindows();

	}
}