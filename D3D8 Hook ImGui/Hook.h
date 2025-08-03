#ifndef HOOK_H
#define HOOK_H

#include "pch.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using tEndScene = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE8 pDevice);
using tReset = HRESULT(APIENTRY*)(LPDIRECT3DDEVICE8 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

class Hook
{
public:
    static IDirect3DDevice8* pDevice;
    static tEndScene oEndScene;
    static tReset oReset;  
    static HWND window;
    static HMODULE hDDLModule;
    static DWORD gamebaseAddress;  

    static void HookDirectX();
    static void UnHookDirectX();
    static void HookWindow();
    static void UnHookWindow();

    static bool HookDirectXViaGamebase();
    static LPDIRECT3DDEVICE8 GetDeviceFromGamebase();
    static int windowHeight, windowWidth;
private:
    static WNDPROC OWndProc;
    static bool windowHooked;
    static void* d3d8Device[119];
    static bool isHooked;

   

    static BOOL GetD3D8Device(void** pTable, size_t size);

    static BOOL CALLBACK enumWind(HWND handle, LPARAM lp);
    static HWND GetProcessWindow();

    static bool AttachDetours();
    static bool DetachDetours();

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static HRESULT APIENTRY hkReset(LPDIRECT3DDEVICE8 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
};

#define DX8_VTABLE_SIZE 119
#define DX8_ENDSCENE_INDEX 35
#define DX8_RESET_INDEX 14

#endif 