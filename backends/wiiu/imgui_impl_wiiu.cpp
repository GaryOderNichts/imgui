// dear imgui: Platform Backend for the Wii U
#include "imgui.h"
#include "imgui_impl_wiiu.h"
#include <stdlib.h> // malloc/free

// Software keyboard
#include <nn/swkbd.h>

// Wii U Data
struct ImGui_ImplWiiU_Data
{
    nn::swkbd::CreateArg CreateArg;
    nn::swkbd::AppearArg AppearArg;

    bool WantedTextInput;
    bool WasTouched;

    ImGui_ImplWiiU_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData
static ImGui_ImplWiiU_Data* ImGui_ImplWiiU_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplWiiU_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;
}

bool     ImGui_ImplWiiU_Init()
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    // Setup backend data
    ImGui_ImplWiiU_Data* bd = IM_NEW(ImGui_ImplWiiU_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_wiiu";

    // Initialize and create software keyboard
    nn::swkbd::CreateArg createArg;

    createArg.workMemory = malloc(nn::swkbd::GetWorkMemorySize(0));
    createArg.fsClient = (FSClient*) malloc(sizeof(FSClient));
    if (!createArg.workMemory || !createArg.fsClient)
    {
        free(createArg.workMemory);
        free(createArg.fsClient);
        return false;
    }

    FSAddClient(createArg.fsClient, FS_ERROR_FLAG_NONE);

    if (!nn::swkbd::Create(createArg))
        return false;

    nn::swkbd::AppearArg appearArg;
    bd->CreateArg = createArg;
    bd->AppearArg = appearArg;

    return true;
}

void     ImGui_ImplWiiU_Shutdown()
{
    ImGui_ImplWiiU_Data* bd = ImGui_ImplWiiU_GetBackendData();
    IM_ASSERT(bd != NULL && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    // Destroy software keyboard
    nn::swkbd::Destroy();
    free(bd->CreateArg.workMemory);
    bd->CreateArg.workMemory = NULL;

    if (bd->CreateArg.fsClient)
    {
        FSDelClient(bd->CreateArg.fsClient, FS_ERROR_FLAG_NONE);
        free(bd->CreateArg.fsClient);
        bd->CreateArg.fsClient = NULL;
    }

    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    IM_DELETE(bd);
}

static void ImGui_ImplWiiU_UpdateKeyboardInput(ImGui_ImplWiiU_ControllerInput* input)
{
    ImGuiIO& io = ImGui::GetIO();

    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &input->vpad->tpNormal, &input->vpad->tpNormal);

    nn::swkbd::ControllerInfo controllerInfo;
    controllerInfo.vpad = input->vpad;
    for (int i = 0; i < 4; i++)
        controllerInfo.kpad[i] = input->kpad[i];

    nn::swkbd::Calc(controllerInfo);

    if (nn::swkbd::IsNeedCalcSubThreadFont())
        nn::swkbd::CalcSubThreadFont();

    if (nn::swkbd::IsNeedCalcSubThreadPredict())
        nn::swkbd::CalcSubThreadPredict();

    if (nn::swkbd::IsDecideOkButton(NULL))
    {
        // Add entered text
        const char16_t* string = nn::swkbd::GetInputFormString();
        for (int i = 0; *string; string++)
            io.AddInputCharacterUTF16(string[i]);

        // close keyboard
        nn::swkbd::DisappearInputForm();
    }

    if (nn::swkbd::IsDecideCancelButton(NULL))
        nn::swkbd::DisappearInputForm();
}

static void ImGui_ImplWiiU_UpdateTouchInput(ImGui_ImplWiiU_ControllerInput* input)
{
    ImGui_ImplWiiU_Data* bd = ImGui_ImplWiiU_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    VPADTouchData touch;
    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &touch, &input->vpad->tpNormal);

    if (touch.touched)
    {
        float scale_x = (io.DisplaySize.x / io.DisplayFramebufferScale.x) / 1280.0f;
        float scale_y = (io.DisplaySize.y / io.DisplayFramebufferScale.y) / 720.0f;
        io.AddMousePosEvent(touch.x * scale_x, touch.y * scale_y);
    }

    if (touch.touched != bd->WasTouched)
    {
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, touch.touched);
        bd->WasTouched = touch.touched;
    }
}

static void ImGui_ImplWiiU_UpdateControllerInput(ImGui_ImplWiiU_ControllerInput* input)
{

}

bool     ImGui_ImplWiiU_ProcessInput(ImGui_ImplWiiU_ControllerInput* input)
{
    ImGui_ImplWiiU_Data* bd = ImGui_ImplWiiU_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplWiiU_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Show keyboard if wanted
    if (io.WantTextInput && !bd->WantedTextInput) 
    {
        if (nn::swkbd::GetStateInputForm() == nn::swkbd::State::Hidden)
            nn::swkbd::AppearInputForm(bd->AppearArg);
    }
    bd->WantedTextInput = io.WantTextInput;

    // Update keyboard input
    if (nn::swkbd::GetStateInputForm() != nn::swkbd::State::Hidden)
    {
        ImGui_ImplWiiU_UpdateKeyboardInput(input);
        return true;
    }

    // Update touch screen
    ImGui_ImplWiiU_UpdateTouchInput(input);

    // Update gamepads
    ImGui_ImplWiiU_UpdateControllerInput(input);

    return false;
}

void     ImGui_ImplWiiU_DrawKeyboardOverlay(bool drawDRC)
{
    if (nn::swkbd::GetStateInputForm() != nn::swkbd::State::Hidden)
    {
        if (drawDRC)
            nn::swkbd::DrawDRC();
        else
            nn::swkbd::DrawTV();
    }
}
