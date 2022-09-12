#include "main.h"

// Style Setting

void PopStyle(int time) {
  for (int i = 0; i < time; i++) ImGui::PopStyleColor();
}

void ImGui::PushStyleTextDisabled() {
  ImGui::PushStyleColor(ImGuiCol_Text,
                        ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
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

// Helper functions

float GetRealWidth(float width, int itemCount, bool hasPadding) {
  return width + (itemCount - 1) * ImGui::GetStyle().ItemSpacing.x +
         (hasPadding ? 4 * ImGui::GetStyle().FramePadding.x : 0);
}

float GetRealHeight(float height, int itemCount, bool hasPadding) {
  return height + (itemCount - 1) * ImGui::GetStyle().ItemSpacing.y +
         (hasPadding ? 4 * ImGui::GetStyle().FramePadding.y : 0);
}
