#include <Windows/Windows.h>
#include <WindowsConfig.h>
#include <ILog.h>

#include <IBufferManager.h> // 初始化管理器

namespace Editor
{

	AUTO_REGISTER_SINGLETON(Windows, Normal)

	Windows::Windows()
	{
		// not
	}

	bool Windows::Init()
	{
		LOG_INFO("Editor subsystem created.");

		// Initialize the editor subsystem, e.g. create windows, initialize resources, etc.
		LOG_INFO("Editor Create Windows.");
		CreateWindows();
		LOG_INFO("Editor Create DeviceD3D.");
		CreateDeviceD3D();

		// init buffer Manager
		LOG_INFO("Init Buffer Manager.");
		RenderCore::IBufferManagerAdmin* IBMAdmin = RenderCore::GetBufferManagerAdminInterface();
		WindowsConfig& config = WindowsConfig::Get();
		RenderCore::ViewContext context;
		context.ScreenSize = XMINT2(config.width, config.height);
		IBMAdmin->Initialize(context);

		return true;
	}

	void Windows::Uninstall()
	{
		CleanupWindows();
	}

	// 注册窗口改变时渲染回调函数
	void Windows::RegisterWindowUpdateRenderCallbackFunction(WindowUpdateRenderingFunc func) {
		this->WUR = func;
	}

	Windows::WindowUpdateRenderingFunc Windows::GetWindowUpdateRenderFunction()
	{
		return this->WUR;
	}
}