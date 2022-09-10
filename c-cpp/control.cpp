#include "game.h"

void Game::NumPadWindow(int *p_num) {
  int num = 0;
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (x != 0) ImGui::SameLine();
      ImGui::PushID(++num);
      if (ImGui::Selectable(numStr[num], *p_num == num, 0,
                            ImVec2(CellSize, CellSize)))
        *p_num = num;
      ImGui::PopID();
    }
  }
  if (ImGui::Selectable("Eraser", *p_num == 0, 0,
                        ImVec2(ImGui::GetRealWidth(3 * CellSize, 3), CellSize)))
    *p_num = 0;
}
