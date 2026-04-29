#pragma once
#include <Windows.h>
#include <Common/Singleton.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>    // 二进制
//#include <cereal/types/string.hpp>     // std::string

namespace Editor {
	// Forward declaration of the window procedure
	LRESULT CALLBACK WindowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	struct WindowsConfig : public Singleton<WindowsConfig>
	{
		int width = 1280;
		int height = 800;

		int windowsX = 100;
		int windowsY = 100;

		template<class Archive>
		void serialize(Archive& archive) {
			archive(width, height, windowsX, windowsY);
		}
	};
}