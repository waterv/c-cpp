#ifndef __MAIN_H__
#define __MAIN_H__

#include <d3d9.h>
#include <math.h>
#include <tchar.h>

#include <fstream>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "yaml-cpp/yaml.h"

// Direct3D helpers
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();

void ResetDevice();

// Win32 message handlers
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam,
                       LPARAM lParam);  // from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

#endif
