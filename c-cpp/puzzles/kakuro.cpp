#include "game.h"

using KakuroOperation = NumberOperation;
extern ImFont* defaultFont;

struct Kakuro : public Puzzle<KakuroOperation> {
  int width, height;
  std::vector<std::vector<int>> clues_right, clues_bottom, board, bordered;

  // std::string errType;  // "same number", "wrong sum", "best"
  int errX, errY;

  Kakuro(const char* id);

  void input(int x, int y, int num);

  void undo_(KakuroOperation op) { board[op.y][op.x] = op.prev; };
  void redo_(KakuroOperation op) { board[op.y][op.x] = op.curr; };
  void clear_() {
    for (auto& u : board) u.clear();
  };

  bool isRelated(int x1, int y1, int x2, int y2) const;
  void check();
};

enum Bordered { Bordered_Right = 1 << 0, Bordered_Bottom = 1 << 1 };

void Game::KakuroWindow(bool* p_open) {
  static Kakuro kakuro{"bachelor_seal/1"};

  if (!ImGui::Begin("Kakuro", p_open,
                    ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_MenuBar))
    return ImGui::End();

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});

  // Menu
  static bool showBorder;

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      if (ImGui::MenuItem("Select Level..", "Ctrl+O"))
        kakuro.showLevelSelect = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Undo", "Ctrl+Z", false, kakuro.canUndo()))
        kakuro.undo();
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false, kakuro.canRedo()))
        kakuro.redo();
      ImGui::Separator();
      if (ImGui::MenuItem("Clear")) kakuro.clear();
      if (ImGui::MenuItem("Reset")) kakuro = Kakuro{kakuro.id.c_str()};
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Show Border", NULL, showBorder))
        showBorder = !showBorder;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("Tutorial")) kakuro.showTutorial = true;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  static int hoveredX = 0;
  static int hoveredY = 0;

  static std::string id;
  if (kakuro.LevelSelectWindow(&id)) {
    kakuro = Kakuro{id.c_str()};
    hoveredX = 0;
    hoveredY = 0;
  }
  kakuro.TutorialWindow();

  // Keyboard & Mouse
  static int num = 1;
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
    for (ImGuiKey key = ImGuiKey_0; key <= ImGuiKey_9; key++)
      if (ImGui::IsKeyDown(key)) num = key - ImGuiKey_0;

    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel > 0)
      num = (num + 10 - 1) % 10;
    else if (wheel < 0)
      num = (num + 1) % 10;
  }

  // Board
  ImGui::BeginChild("KakuroChildL",
                    {GetRealWidth(CellSize, kakuro.width, true),
                     GetRealHeight(CellSize, kakuro.height, true)},
                    true);
  const auto pos = ImGui::GetCursorScreenPos();
  const auto col = ImGui::BorderColor();
  const auto bac = ImGui::BackgroundColor();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  for (int y = 0; y <= kakuro.height; y++)
    for (int x = 0; x <= kakuro.width; x++) {
      const auto vec = ImGui::GetPos(pos, x, y);

      if (kakuro.bordered[y][x] & Bordered_Right)
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right, col, 3.0f);
      if (kakuro.bordered[y][x] & Bordered_Bottom)
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down, col, 3.0f);

      if (x == 0 || y == 0) continue;
      if (x != 1) ImGui::SameLine();

      ImGui::PushID(x + y * kakuro.width);
      if (kakuro.clues_right[y][x] != 0) {
        ImGui::DrawRectFilled(drawList, vec);
        ImGui::DrawLine(drawList, vec);
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right);
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down);
        ImGui::PushFont(defaultFont);
        if (kakuro.clues_right[y][x] != -1) {
          drawList->AddText({vec.z - 15.0f, vec.y}, col,
                            std::to_string(kakuro.clues_right[y][x]).c_str());
        }
        if (kakuro.clues_bottom[y][x] != -1)
          drawList->AddText({vec.x + 2.0f, vec.w - 13.0f}, col,
                            std::to_string(kakuro.clues_bottom[y][x]).c_str());
        ImGui::PopFont();
        ImGui::Selectable("", kakuro.isRelated(hoveredX, hoveredY, x, y), 0,
                          ImVec2(CellSize, CellSize));
      } else {
        if (showBorder) {
          ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right, bac);
          ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down, bac);
        }
        bool isNote = kakuro.board[y][x] > 9;
        if (isNote) ImGui::PushStyleTextPlotHistogram();
        if (ImGui::Selectable(
                kakuro.board[y][x] % 10
                    ? std::to_string(kakuro.board[y][x] % 10).c_str()
                    : "",
                kakuro.isRelated(hoveredX, hoveredY, x, y), 0,
                ImVec2(CellSize, CellSize))) {
          kakuro.input(x, y, num);
        }
        if (isNote) ImGui::PopStyleColor();
      }
      if (ImGui::IsItemHovered()) {
        hoveredX = x;
        hoveredY = y;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) ||
            ImGui::IsKeyReleased(ImGuiKey_Minus)) {
          kakuro.input(x, y, num + 10);
        } else if (ImGui::IsKeyReleased(ImGuiKey_Backspace)) {
          kakuro.input(x, y, 0);
        }
      }
      ImGui::PopID();
    }
  ImGui::EndChild();
  ImGui::SameLine();
  ImGui::NumPadChild("KakuroChildR", &num);

  kakuro.LevelDetails(" <There is %s at (%d, %d)>", kakuro.errType.c_str(),
                      kakuro.errX, kakuro.errY);
  ImGui::PopStyleVar();
  ImGui::End();
}

Kakuro::Kakuro(const char* id) : Puzzle{"kakuro", id}, errX{0}, errY{0} {
  width = node["width"].as<int>();
  height = node["height"].as<int>();

  std::vector<int> emptyRow{};
  for (int x = 0; x <= width; x++) emptyRow.push_back(0);
  std::vector<int> isBlockedCol{emptyRow};

  for (int y = -1; y < height; y++) {
    clues_right.push_back(emptyRow);
    clues_bottom.push_back(emptyRow);
    board.push_back(emptyRow);
    bordered.push_back(emptyRow);
    if (y == -1) continue;

    int isBlockedRow = 0;
    for (int x = 0; x < width; x++) {
      int right = node["right"][y][x].as<int>();
      int bottom = node["bottom"][y][x].as<int>();
      clues_right[y + 1][x + 1] = right;
      clues_bottom[y + 1][x + 1] = bottom;
      board[y + 1][x + 1] = 0;
      if (!!isBlockedRow ^ !!right) {
        bordered[y + 1][x] |= Bordered_Right;
        isBlockedRow = 1;
      }
      if (!!isBlockedCol[x + 1] ^ !!right) {
        bordered[y][x + 1] |= Bordered_Bottom;
        isBlockedCol[x + 1] = 1;
      }
      if (right) {
        if (x == width - 1) bordered[y + 1][x + 1] |= Bordered_Right;
        if (y == height - 1) bordered[y + 1][x + 1] |= Bordered_Bottom;
      } else {
        isBlockedRow = isBlockedCol[x + 1] = 0;
      }
    }
  }
}

void Kakuro::input(int x, int y, int num) {
  if (board[y][x] == num) return;
  if (clues_right[y][x] != 0) return;
  if (endTime) return;
  pushHistory({x, y, board[y][x], num});
  board[y][x] = num;
}

bool Kakuro::isRelated(int x1, int y1, int x2, int y2) const {
  if (x1 == x2) {
    for (int y = MIN(y1, y2) + 1; y <= MAX(y1, y2); y++)
      if (clues_right[y][x1]) return false;
  } else if (y1 == y2) {
    for (int x = MIN(x1, x2) + 1; x <= MAX(x1, x2); x++)
      if (clues_right[y1][x]) return false;
  } else
    return false;
  return true;
}

void Kakuro::check() {
  err = true;
  for (int y = 1; y <= height; y++)
    for (int x = 1; x <= width; x++) {
      errX = x;
      errY = y;
      if (clues_right[y][x] > 0) {
        int set = 0, sum = 0;
        for (int xt = x + 1; xt <= width; xt++) {
          if (clues_right[y][xt] != 0) break;
          int num = board[y][xt];
          if (num == 0 || num > 9) {
            errType = "empty cell";
            errX = xt;
            return;
          }
          if ((set | (1 << num)) == set) {
            errType = "duplicated number";
            return;
          }
          set |= (1 << num);
          sum += num;
        }
        if (sum != clues_right[y][x]) {
          errType = "wrong sum";
          return;
        }
      }
      if (clues_bottom[y][x] > 0) {
        int set = 0, sum = 0;
        for (int yt = y + 1; yt <= height; yt++) {
          if (clues_right[yt][x] != 0) break;
          int num = board[yt][x];
          if (num == 0 || num > 9) {
            errType = "empty cell";
            errY = yt;
            return;
          }
          if ((set | (1 << num)) == set) {
            errType = "duplicated number";
            return;
          }
          set |= (1 << num);
          sum += num;
        }
        if (sum != clues_bottom[y][x]) {
          errType = "wrong sum";
          return;
        }
      }
    }
  return win();
}
