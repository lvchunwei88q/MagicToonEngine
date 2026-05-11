#include <Component/DetailsWindow.h>

namespace RenderUI {
	void DetailsWindow() // main Draw
	{
        if (Switch.DetailsWindow)
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Begin("Details Panel");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
	}
}