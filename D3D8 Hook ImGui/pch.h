#ifndef PCH_H
#define PCH_H

#include <Windows.h>
#include "Detours/include/detours.h"
#include "d3d8.h"
#include "d3dx8.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx8.h"
#include "ImGui/imgui_impl_win32.h"
#include "Features/offsets.h"
#include "lazyimporter.h"
#define c(x) LI_FN(x).get()

#endif


