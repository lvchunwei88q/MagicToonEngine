#include <GuiInterface.h>

namespace RenderUI {
	void BasicLayout()
	{
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }
	}
}