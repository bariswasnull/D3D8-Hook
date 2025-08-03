#include "Hook.h"
#include "Drawing.h"

LPDIRECT3DDEVICE8 Hook::pDevice = nullptr;
tEndScene Hook::oEndScene = nullptr;
tReset Hook::oReset = nullptr;  
HWND Hook::window = nullptr;
HMODULE Hook::hDDLModule = nullptr;
DWORD Hook::gamebaseAddress = 0;


int Hook::windowHeight = 0;
int Hook::windowWidth = 0;
WNDPROC Hook::OWndProc = nullptr;
bool Hook::windowHooked = false;

void* Hook::d3d8Device[119];
bool Hook::isHooked = false;



void Hook::HookDirectX()
{
    if (HookDirectXViaGamebase()) {
        return;
    }

   if (GetD3D8Device(d3d8Device, sizeof(d3d8Device)))
   {
       oEndScene = (tEndScene)d3d8Device[DX8_ENDSCENE_INDEX];
       oReset = (tReset)d3d8Device[DX8_RESET_INDEX];

       if (oEndScene && oReset &&
           !IsBadCodePtr((FARPROC)oEndScene) &&
           !IsBadCodePtr((FARPROC)oReset)) {

           if (AttachDetours()) {
               isHooked = true;
           }
       }
   }
}

bool Hook::HookDirectXViaGamebase()
{
    gamebaseAddress = (DWORD)GetModuleHandleA("metin2client.exe");
    if (!gamebaseAddress) {
        gamebaseAddress = (DWORD)GetModuleHandleA("metin2client");
    }
    if (!gamebaseAddress) {
        gamebaseAddress = (DWORD)GetModuleHandleA(NULL);
    }

    if (!gamebaseAddress) {
        return false;
    }

    LPDIRECT3DDEVICE8 pGameDevice = GetDeviceFromGamebase();
    if (!pGameDevice) {
        return false;
    }

    void** vtable = *(void***)(pGameDevice);
    if (!vtable) {
        return false;
    }

    oEndScene = (tEndScene)vtable[DX8_ENDSCENE_INDEX];
    oReset = (tReset)vtable[DX8_RESET_INDEX];

    if (!oEndScene || !oReset ||
        IsBadCodePtr((FARPROC)oEndScene) ||
        IsBadCodePtr((FARPROC)oReset)) {
        return false;
    }

    pDevice = pGameDevice;

    if (AttachDetours()) {
        isHooked = true;
        return true;
    }

    return false;
}

LPDIRECT3DDEVICE8 Hook::GetDeviceFromGamebase()
{
    if (!gamebaseAddress) {
        return nullptr;
    }

    __try {
        LPDIRECT3DDEVICE8* devicePtr = (LPDIRECT3DDEVICE8*)(gamebaseAddress + offsets->DEVICE_OFFSET);
        if (IsBadReadPtr(devicePtr, sizeof(LPDIRECT3DDEVICE8))) {
            return nullptr;
        }

        LPDIRECT3DDEVICE8 device = *devicePtr;

        if (IsBadReadPtr(device, sizeof(void*))) {
            return nullptr;
        }

        void** vtable = *(void***)(device);
        if (IsBadReadPtr(vtable, sizeof(void*) * 50)) {
            return nullptr;
        }

        return device;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

bool Hook::AttachDetours()
{
    DWORD oldProtectEndScene, oldProtectReset;
    if (!VirtualProtect(oEndScene, 8, PAGE_EXECUTE_READWRITE, &oldProtectEndScene)) {
        return false;
    }

    if (!VirtualProtect(oReset, 8, PAGE_EXECUTE_READWRITE, &oldProtectReset)) {
        VirtualProtect(oEndScene, 8, oldProtectEndScene, &oldProtectEndScene);
        return false;
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    LONG attachEndScene = DetourAttach(&(PVOID&)oEndScene, Drawing::hkEndScene);
    if (attachEndScene != NO_ERROR) {
        DetourTransactionAbort();
        VirtualProtect(oEndScene, 8, oldProtectEndScene, &oldProtectEndScene);
        VirtualProtect(oReset, 8, oldProtectReset, &oldProtectReset);
        return false;
    }

    LONG attachReset = DetourAttach(&(PVOID&)oReset, hkReset);
    if (attachReset != NO_ERROR) {
        DetourTransactionAbort();
        VirtualProtect(oEndScene, 8, oldProtectEndScene, &oldProtectEndScene);
        VirtualProtect(oReset, 8, oldProtectReset, &oldProtectReset);
        return false;
    }

    LONG result = DetourTransactionCommit();
    if (result != NO_ERROR) {
        VirtualProtect(oEndScene, 8, oldProtectEndScene, &oldProtectEndScene);
        VirtualProtect(oReset, 8, oldProtectReset, &oldProtectReset);
        return false;
    }
    return true;
}

bool Hook::DetachDetours()
{
    if (!isHooked) {
        return true;
    }

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)oEndScene, Drawing::hkEndScene);
    DetourDetach(&(PVOID&)oReset, hkReset);
    LONG result = DetourTransactionCommit();

    if (result == NO_ERROR) {
        isHooked = false;
        return true;
    }
    return false;
}

void Hook::UnHookDirectX()
{
    if (Drawing::bInit)
    {
        UnHookWindow();
        ImGui_ImplDX8_Shutdown();
        ImGui_ImplWin32_Shutdown();
       
    }

    Drawing::bInit = FALSE;
    DetachDetours();
}

BOOL CALLBACK Hook::enumWind(const HWND handle, LPARAM lp)
{
    DWORD procID;
    GetWindowThreadProcessId(handle, &procID);
    if (GetCurrentProcessId() != procID)
        return TRUE;

    window = handle;
    return FALSE;
}

HWND Hook::GetProcessWindow()
{
    window = nullptr;
    EnumWindows(enumWind, NULL);

    if (!window)
        return nullptr;

    RECT size;
    GetWindowRect(window, &size);
    windowWidth = size.right - size.left;
    windowHeight = size.bottom - size.top;

    windowHeight -= 29;
    windowWidth -= 5;

    return window;
}

BOOL Hook::GetD3D8Device(void** pTable, const size_t size)
{
    if (!pTable) {
        return FALSE;
    }

    IDirect3D8* pD3D = Direct3DCreate8(D3D_SDK_VERSION);
    if (!pD3D) {
        return FALSE;
    }

    HWND hwnd = GetProcessWindow();
    if (!hwnd) {
        hwnd = GetDesktopWindow();
    }

    D3DDISPLAYMODE displayMode;
    if (FAILED(pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode))) {
        pD3D->Release();
        return FALSE;
    }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.BackBufferWidth = 800;
    d3dpp.BackBufferHeight = 600;
    d3dpp.BackBufferFormat = displayMode.Format;
    d3dpp.BackBufferCount = 1;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hwnd;
    d3dpp.Windowed = TRUE;
    d3dpp.EnableAutoDepthStencil = FALSE;
    d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    IDirect3DDevice8* pDummyDevice = nullptr;

    HRESULT deviceResult = pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING,
        &d3dpp,
        &pDummyDevice
    );

    if (FAILED(deviceResult)) {
        deviceResult = pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_REF,
            hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
            &d3dpp,
            &pDummyDevice
        );
    }

    if (FAILED(deviceResult) || !pDummyDevice) {
        pD3D->Release();
        return FALSE;
    }

    BOOL copySuccess = FALSE;
    __try {
        memcpy(pTable, *(void***)(pDummyDevice), size);
        copySuccess = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        copySuccess = FALSE;
    }

    pDummyDevice->Release();
    pD3D->Release();

    return copySuccess;
}

void Hook::HookWindow()
{
    if (!window) {
        window = GetProcessWindow();
    }

    if (window && IsWindow(window)) {
        OWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
        windowHooked = (OWndProc != nullptr);
    }
}

void Hook::UnHookWindow()
{
    if (windowHooked && window && OWndProc) {
        SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)OWndProc);
        windowHooked = false;
    }
}

LRESULT WINAPI Hook::WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
    if (Drawing::bDisplay && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        return true;
    }

    if (Drawing::bInit)
        ImGui::GetIO().MouseDrawCursor = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

    if (msg == WM_CLOSE)
    {
        UnHookDirectX();
        UnHookWindow();
        TerminateProcess(GetCurrentProcess(), 0);
    }

    if (ImGui::GetIO().WantCaptureMouse)
    {
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
            return true;
        return false;
    }

    return CallWindowProc(OWndProc, hWnd, msg, wParam, lParam);
}

HRESULT Hook::hkReset(LPDIRECT3DDEVICE8 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX8_InvalidateDeviceObjects();
    HRESULT tmpReset = oReset(pDevice, pPresentationParameters);
    ImGui_ImplDX8_CreateDeviceObjects();
    return tmpReset;
}