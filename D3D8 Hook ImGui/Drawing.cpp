#define IMGUI_DEFINE_MATH_OPERATORS
#include "Drawing.h"
#include "Hook.h"
#include "Features/MemoryFunctions.h"
#include "Detours/skCrypt.h"
#include "Features/FeatureSettings.h"

BOOL Drawing::bInit = FALSE;
bool Drawing::bDisplay = true;
bool Drawing::bSetPos = false;
ImVec2 Drawing::vWindowPos = { 0, 0 };
ImVec2 Drawing::vWindowSize = { 0, 0 };

void Drawing::RenderMainMenu()
{
    const ImVec2 menuSize = ImVec2(350, 300);
    const ImVec2 screenSize = ImGui::GetIO().DisplaySize;
    const ImVec2 menuPos = ImVec2((screenSize.x - menuSize.x) * 0.5f, (screenSize.y - menuSize.y) * 0.5f);

    ImGui::SetNextWindowPos(menuPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(menuSize, ImGuiCond_FirstUseEver);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Damage Hack", &bDisplay, window_flags))
    {
        ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "v1.0");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "t72reformed");

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Ana Ayarlar", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Columns(2, nullptr, false);
            ImGui::SetColumnWidth(0, 160);

            ImGui::Checkbox("7x Hasar", &FeatureSettings::_7xActivate);
            ImGui::Checkbox("Dummy Mode", &FeatureSettings::isDummy);
            ImGui::Checkbox("Bow Mode", &FeatureSettings::isBow);

            ImGui::NextColumn();

            ImGui::Text("Mesafe:");
            ImGui::SliderInt("##dist", &FeatureSettings::attackDistance, 50, 1500, "%d");
            ImGui::Text("Delay:");
            ImGui::SliderInt("##delay", &FeatureSettings::attackDelay, 1, 300, "%dms");
            ImGui::SliderInt("Mob Size", &FeatureSettings::mobSize, 1, 20, "%d");

            ImGui::Columns(1);
        }

        if (ImGui::CollapsingHeader("Hedefler"))
        {
            ImGui::BeginGroup();
            ImGui::Checkbox("Player", &FeatureSettings::targetPlayer);
            ImGui::SameLine();
            ImGui::Checkbox("Mob", &FeatureSettings::targetMob);
            ImGui::SameLine();
            ImGui::Checkbox("Metin", &FeatureSettings::targetMetin);
            ImGui::EndGroup();
        }

        if (ImGui::CollapsingHeader("Durum"))
        {
            ImGui::Text("Mod: %s", FeatureSettings::isDummy ? "Dummy" : "Ana");
            ImGui::Text("Aktif: %s", FeatureSettings::_7xActivate ? "Evet" : "Hayır");
            ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
        }

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 35);
        ImGui::Separator();

        if (ImGui::Button("Gizle", ImVec2(60, 25)))
        {
            bDisplay = false;
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 70);
        if (ImGui::Button("Çıkış", ImVec2(60, 25)))
        {
            Hook::UnHookDirectX();
            CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, Hook::hDDLModule, 0, nullptr);
        }
    }
    ImGui::End();
}

HRESULT Drawing::hkEndScene(const LPDIRECT3DDEVICE8 D3D8Device)
{
    if (!Hook::pDevice)
        Hook::pDevice = D3D8Device;

    if (!bInit)
        InitImGui(D3D8Device);

    if (GetAsyncKeyState(VK_INSERT) & 1)
        bDisplay = !bDisplay;

    if (GetAsyncKeyState(VK_END) & 1)
    {
        Hook::UnHookDirectX();
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, Hook::hDDLModule, 0, nullptr);
        return Hook::oEndScene(D3D8Device);
    }

    ImGui_ImplDX8_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (bDisplay)
    {
        RenderMainMenu();
    }

    RenderOverlay();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX8_RenderDrawData(ImGui::GetDrawData());
    return Hook::oEndScene(D3D8Device);
}

void Drawing::InitImGui(const LPDIRECT3DDEVICE8 pDevice)
{
    D3DDEVICE_CREATION_PARAMETERS CP;
    pDevice->GetCreationParameters(&CP);
    Hook::window = CP.hFocusWindow;
    Hook::HookWindow();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(Hook::window);
    ImGui_ImplDX8_Init(pDevice);

    bInit = TRUE;
}

void Drawing::RenderOverlay()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.8f);

    ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin("##Overlay", nullptr, overlay_flags))
    {
        ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Damage Hack");

        if (FeatureSettings::_7xActivate) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "AKTIF");
            ImGui::SameLine();
            ImGui::Text("| %s", FeatureSettings::isDummy ? "Dummy" : "Ana");
        }
        else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "KAPALI");
        }

        // Sadece aktif olanları göster
        if (FeatureSettings::targetPlayer || FeatureSettings::targetMob || FeatureSettings::targetMetin) {
            ImGui::Text("Hedef: ");
            ImGui::SameLine();
            bool first = true;
            if (FeatureSettings::targetPlayer) {
                if (!first) ImGui::SameLine();
                ImGui::Text("P");
                first = false;
            }
            if (FeatureSettings::targetMob) {
                if (!first) ImGui::SameLine();
                ImGui::Text("M");
                first = false;
            }
            if (FeatureSettings::targetMetin) {
                if (!first) ImGui::SameLine();
                ImGui::Text("Mt");
            }
        }
    }
    ImGui::End();
}