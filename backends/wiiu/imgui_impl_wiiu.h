// dear imgui: Platform Backend for the Wii U
#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

// GamePad Input
#include <vpad/input.h>
// Controller Input
#include <padscore/kpad.h>

struct ImGui_ImplWiiU_ControllerInput
{
   VPADStatus* vpad = nullptr;
   KPADStatus* kpad[4] = { nullptr };
};

IMGUI_IMPL_API bool     ImGui_ImplWiiU_Init();
IMGUI_IMPL_API void     ImGui_ImplWiiU_Shutdown();
IMGUI_IMPL_API bool     ImGui_ImplWiiU_ProcessInput(ImGui_ImplWiiU_ControllerInput* input);
IMGUI_IMPL_API void     ImGui_ImplWiiU_DrawKeyboardOverlay(bool drawDRC);
