#include "Main.h"
#include <IProjectController.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CREATE: // 窗口创建
	{
		init_app(hwnd);
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

void Operation(JSON json)
{
	std::string action = json["action"];
	if (action == "create_projnew_projectct") {
		std::string name = json["name"];
		std::string path = json["path"];
		NewProJect(path,name);
	}
}

using namespace EngineProject;

void NewProJect(std::string path, std::string name)
{
	IProjectController* controller = GetProjectControllerInterface();

	ProJectConfig config;
	config.name = name;
	config.path = path;
	controller->Create(config);
}