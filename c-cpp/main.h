#ifndef __MAIN_H__
#define __MAIN_H__

#include <d3d9.h>
#include <math.h>
#include <tchar.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "yaml-cpp/yaml.h"

const int CellSize = 25;

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

// Misc functions
namespace ImGui {
void PushStyleTextDisabled();
void PushStyleButtonColored(int idx, int max);
void PopStyleButtonColored();

void TimeText(int time);
void TimeText(float time);
void ErrorText(const char* fmt, ...);
void Difficulty(int difficulty);
bool BeginCenterPopupModal(const char title[], bool* p_open);

float GetRealWidth(float width, int itemCount, bool hasPadding = false);
float GetRealHeight(float height, int itemCount, bool hasPadding = false);
}  // namespace ImGui

#endif
