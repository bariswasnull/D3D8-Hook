#ifndef DRAWING_H
#define DRAWING_H
#include "pch.h"

class Drawing
{
public:
    static bool bDisplay;
    static BOOL bInit;
    static bool bSetPos;

    static HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE8 D3D8Device);

    static void RenderMainMenu();
    static void RenderOverlay();


private:
    static ImVec2 vWindowPos;
    static ImVec2 vWindowSize;

    static void InitImGui(LPDIRECT3DDEVICE8 pDevice);
};

#endif