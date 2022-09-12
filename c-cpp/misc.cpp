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

// Customized Components

void ImGui::TimeText(int time) {
  ImGui::Text("%02d:%02d", time / 60, time % 60);
}

void ImGui::TimeText(float time) { ImGui::TimeText((int)time); }

void ImGui::ErrorText(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.0f, 0.0f, 1.0f});
  ImGui::TextV(fmt, args);
  ImGui::PopStyleColor();
  va_end(args);
}

bool ImGui::Difficulty(int difficulty) {
  static char difficulties[4][10]{"Easy", "Medium", "Hard", "V.Hard"};
  ImGui::PushStyleButtonColored(3 - difficulty, 7);
  bool pressed = ImGui::Button(difficulties[difficulty], {DifficultySize, 0});
  ImGui::PopStyleButtonColored();
  return pressed;
}

// Customized Windows

void ImGui::NumPadWindow(int* p_num) {
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
                        ImVec2(GetRealWidth(3 * CellSize, 3), CellSize)))
    *p_num = 0;
}

// Helper functions

float GetRealWidth(float width, int itemCount, bool hasPadding) {
  return width + (itemCount - 1) * ImGui::GetStyle().ItemSpacing.x +
         (hasPadding ? 4 * ImGui::GetStyle().FramePadding.x : 0);
}

float GetRealHeight(float height, int itemCount, bool hasPadding) {
  return height + (itemCount - 1) * ImGui::GetStyle().ItemSpacing.y +
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
