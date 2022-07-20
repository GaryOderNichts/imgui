// Dear ImGui: standalone example application for the Wii U

// ImGui includes
#include "imgui.h"
#include "imgui_impl_gx2.h"

// Graphics includes
#include <whb/gfx.h>
#include <whb/proc.h>
#include <gx2/registers.h>
#include <gx2/swap.h>

// Input includes
#include <vpad/input.h>

int main(int, char**)
{
    // Init ProcUI and GX2
    WHBProcInit();
    WHBGfxInit();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Renderer backends
    ImGui_ImplGX2_Init();

    // Our state
    bool was_touched = false;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Setup display sizes and scales
    io.DisplaySize.x = (float)WHBGfxGetTVColourBuffer()->surface.width; // set the current display width
    io.DisplaySize.y = (float)WHBGfxGetTVColourBuffer()->surface.height; // set the current display height here
    io.DisplayFramebufferScale = ImVec2(2.0f, 2.0f);

    // Main loop
    while (WHBProcIsRunning())
    {
        // Start a new frame / We'll be using the TV buffer
        WHBGfxBeginRender();
        WHBGfxBeginRenderTV();

        VPADStatus vpad;
        VPADRead(VPAD_CHAN_0, &vpad, 1, nullptr);

        VPADTouchData touch;
        VPADGetTPCalibratedPoint(VPAD_CHAN_0, &touch, &vpad.tpNormal);

        if (touch.touched) {
            float scale_x = (io.DisplaySize.x / io.DisplayFramebufferScale.x) / 1280.0f;
            float scale_y = (io.DisplaySize.y / io.DisplayFramebufferScale.y) / 720.0f;
            io.AddMousePosEvent(touch.x * scale_x, touch.y * scale_y);
        }

        if (touch.touched != was_touched) {
            io.AddMouseButtonEvent(0, touch.touched);
            was_touched = touch.touched;
        }

        // Start the Dear ImGui frame
        ImGui_ImplGX2_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        float display_w = (float)WHBGfxGetTVColourBuffer()->surface.width;
        float display_h = (float)WHBGfxGetTVColourBuffer()->surface.height;
        GX2SetViewport(0, 0, display_w, display_h, 1.0f, 0.0f);
        WHBGfxClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);

        ImGui_ImplGX2_RenderDrawData(ImGui::GetDrawData());

        WHBGfxFinishRenderTV();
        // copy the tv buffer to the drc as well
        GX2CopyColorBufferToScanBuffer(WHBGfxGetTVColourBuffer(), GX2_SCAN_TARGET_DRC);

        WHBGfxFinishRender();
    }

    // Cleanup
    ImGui_ImplGX2_Shutdown();
    ImGui::DestroyContext();

    WHBGfxShutdown();
    WHBProcShutdown();

    return 0;
}
