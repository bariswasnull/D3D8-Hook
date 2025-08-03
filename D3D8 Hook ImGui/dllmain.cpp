#include "Hook.h"
#pragma once
#define WIN32_LEAN_AND_MEAN 
#include "Features/PlayerFunctions.h"
PlayerFunctions* g_MobberSystem = nullptr;
#pragma once



DWORD WINAPI InitializeMobberSystem(LPVOID lpParam) {
    
    Sleep(2000);
    inithelper();
    offsets_t* pOffsets = gethelper();
    offsets = pOffsets;
    try {
       
        g_MobberSystem = new PlayerFunctions();
        g_MobberSystem->StartMobberSystem();
      
    }
    catch (...) {
      
        if (g_MobberSystem) {
            delete g_MobberSystem;
            g_MobberSystem = nullptr;
        }
    }

    return 0;
}

void CleanupMobberSystem() {
    if (g_MobberSystem) {
       
        g_MobberSystem->StopMobberSystem();
        delete g_MobberSystem;
        g_MobberSystem = nullptr;
       
    }
}
BOOL WINAPI DllMain(const HINSTANCE hinstDLL, const DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		Hook::hDDLModule = hinstDLL;
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Hook::HookDirectX, nullptr, 0, nullptr);
        CreateThread(nullptr, 0, InitializeMobberSystem, nullptr, 0, nullptr);
	}

    if (fdwReason == DLL_PROCESS_DETACH)
    {
        CleanupMobberSystem();
        Hook::UnHookDirectX();
    }

	return TRUE;
}

