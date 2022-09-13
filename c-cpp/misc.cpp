#include "game.h"

// Style Settings

void PopStyle(int time) {
  for (int i = 0; i < time; i++) ImGui::PopStyleColor();
}

void ImGui::PushStyleTextDisabled() {
  ImGui::PushStyleColor(ImGuiCol_Text,
                        ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
}

void ImGui::PushStyleTextPlotHistogram() {
  ImGui::PushStyleColor(ImGuiCol_Text,
                        ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
}

void ImGui::PushStyleButtonColored(int idx, int max) {
  float val = (float)idx / max;
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(val, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        (ImVec4)ImColor::HSV(val, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        (ImVec4)ImColor::HSV(val, 0.8f, 0.8f));
}

void ImGui::PopStyleButtonColored() { PopStyle(3); }

ImU32 ImGui::BorderColor() {
  ImU32 result = ImColor{ImGui::GetStyleColorVec4(ImGuiCol_Text)};
  return result;
}

ImU32 ImGui::BackgroundColor(float alpha) {
  auto color = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
  color.w = alpha;
  ImU32 result = ImColor{color};
  return result;
}

ImVec4 ImGui::GetPos(ImVec2 screenPos, int x, int y) {
  auto pad = ImGui::GetStyle().ItemSpacing;
  float x1 = screenPos.x + pad.x / 2 + GetRealWidth(CellSize, x - 1);
  float y1 = screenPos.y + pad.y / 2 + GetRealHeight(CellSize, y - 1);
  float x2 = pad.x + x1 + CellSize;
  float y2 = pad.y + y1 + CellSize;
  return {x1, y1, x2, y2};
}

void ImGui::DrawLine(ImDrawList* drawList, ImVec4 pos, ImU32 col,
                     float thickness) {
  drawList->AddLine({pos.x, pos.y}, {pos.z, pos.w}, col, thickness);
}

void ImGui::DrawRectFilled(ImDrawList* drawList, ImVec4 pos, ImU32 col) {
  drawList->AddRectFilled({pos.x, pos.y}, {pos.z, pos.w}, col);
}

void ImGui::DrawBorder(ImDrawList* drawList, ImVec2 screenPos, int x, int y,
                       ImGuiDir dir, ImU32 col, float thickness) {
  auto pos = ImGui::GetPos(screenPos, x, y);
  float o = thickness / 2;  // Offset
  switch (dir) {
    case ImGuiDir_Left:
      return drawList->AddLine({pos.x, pos.y - o}, {pos.x, pos.w + o}, col,
                               thickness);
    case ImGuiDir_Right:
      return drawList->AddLine({pos.z, pos.y - o}, {pos.z, pos.w + o}, col,
                               thickness);
    case ImGuiDir_Up:
      return drawList->AddLine({pos.x - o, pos.y}, {pos.z + o, pos.y}, col,
                               thickness);
    case ImGuiDir_Down:
      return drawList->AddLine({pos.x - o, pos.w}, {pos.z + o, pos.w}, col,
                               thickness);
  }
}

// Customized Components

void ImGui::TimeText(int time) {
  ImGui::Text("%02d:%02d", time / 60, time % 60);
}

void ImGui::TimeText(float time) { ImGui::TimeText((int)time); }

bool ImGui::Difficulty(int difficulty) {
  static char difficulties[4][10]{"Easy", "Medium", "Hard", "V.Hard"};
  ImGui::PushStyleButtonColored(3 - difficulty, 7);
  bool pressed = ImGui::Button(difficulties[difficulty], {DifficultySize, 0});
  ImGui::PopStyleButtonColored();
  return pressed;
}

// Customized Windows

void ImGui::NumPadChild(const char* id, int* p_num) {
  ImGui::BeginChild(
      id, {GetRealWidth(CellSize, 3, true), GetRealHeight(CellSize, 4, true)},
      true);
  int num = 0;
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (x != 0) ImGui::SameLine();
      ImGui::PushID(++num);
      if (ImGui::Selectable(std::to_string(num).c_str(), *p_num == num, 0,
                            ImVec2(CellSize, CellSize)))
        *p_num = num;
      ImGui::PopID();
    }
  }
  if (ImGui::Selectable("Eraser", *p_num == 0, 0,
                        ImVec2(GetRealWidth(CellSize, 3), CellSize)))
    *p_num = 0;
  ImGui::EndChild();
}

// Helper functions

float GetRealWidth(float width, int itemCount, bool hasPadding) {
  return itemCount * width + (itemCount - 1) * ImGui::GetStyle().ItemSpacing.x +
         (hasPadding ? 4 * ImGui::GetStyle().FramePadding.x : 0);
}

float GetRealHeight(float height, int itemCount, bool hasPadding) {
  return itemCount * height +
         (itemCount - 1) * ImGui::GetStyle().ItemSpacing.y +
         (hasPadding ? 4 * ImGui::GetStyle().FramePadding.y : 0);
}

std::vector<std::string> split(std::string str, char delim) {
  std::vector<std::string> result;
  size_t previous = 0;
  size_t current = str.find(delim);
  while (current != std::string::npos) {
    if (current > previous) {
      result.push_back(str.substr(previous, current - previous));
    }
    previous = current + 1;
    current = str.find(delim, previous);
  }
  if (previous != str.size()) {
    result.push_back(str.substr(previous));
  }
  return result;
}
