#include "game.h"

class Sudoku {
 public:
  std::string id;
  std::string author;
  int difficulty;

  int clues[9][9];
  int board[9][9];

  float startTime;
  float endTime;
  float bestTime;

  bool err;
  char errtype[7];  // 取值有 "row", "column", "block", "best".
  int erridx;

  Sudoku(const char *filename);
  void clear();
  void check();
};

enum HighlightMode {
  HighlightMode_Blocks,
  HighlightMode_RelatedCells,
  HighlightMode_SameNumbers,
  HighlightModes,
};

void Game::SudokuWindow(bool *p_open) {
  static Sudoku sudoku{"psv/1"};

  if (!ImGui::Begin("Sudoku", p_open,
                    ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_MenuBar)) {
    ImGui::End();
    return;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});

  // Menu
  static bool showSelectLevel = false;
  static bool showTutorialWindow = false;
  static int highlightMode = HighlightMode::HighlightMode_Blocks;
  bool highlightModeBools[HighlightMode::HighlightModes];
  for (int i = 0; i < HighlightMode::HighlightModes; i++)
    highlightModeBools[i] = highlightMode == i;

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      showSelectLevel = ImGui::MenuItem("Select Level..");
      if (ImGui::MenuItem("Clear")) sudoku.clear();
      if (ImGui::MenuItem("Reset")) sudoku = Sudoku{sudoku.id.c_str()};
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::MenuItem("Highlight", NULL, false, false);
      if (ImGui::MenuItem("3x3 Blocks", "Ctrl+1", highlightModeBools))
        highlightMode = HighlightMode::HighlightMode_Blocks;
      if (ImGui::MenuItem("Related Cells", "Ctrl+2", highlightModeBools + 1))
        highlightMode = HighlightMode::HighlightMode_RelatedCells;
      if (ImGui::MenuItem("Same Numbers", "Ctrl+3", highlightModeBools + 2))
        highlightMode = HighlightMode::HighlightMode_SameNumbers;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      showTutorialWindow = ImGui::MenuItem("Tutorial");
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Level Select
  static std::string levelSelected;
  if (showSelectLevel)
    if (Game::LevelSelectWindow("sudoku", &showSelectLevel, &levelSelected))
      sudoku = Sudoku{levelSelected.c_str()};

  if (showTutorialWindow) Game::TutorialWindow("sudoku", &showTutorialWindow);

  // Select highlight mode / number by keyboard
  static int num = 1;
  if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
    for (ImGuiKey key = ImGuiKey_1; key <= ImGuiKey_3; key++)
      if (ImGui::IsKeyDown(key)) highlightMode = key - ImGuiKey_1;
  } else {
    for (ImGuiKey key = ImGuiKey_0; key <= ImGuiKey_9; key++)
      if (ImGui::IsKeyDown(key)) num = key - ImGuiKey_0;
  }

  // Board
  ImGui::BeginChild("SudokuChildL",
                    {GetRealWidth(9 * CellSize, 9, true),
                     GetRealHeight(9 * CellSize, 9, true)},
                    true);
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      bool disabled = (bool)sudoku.clues[y][x];
      bool highlighted;

      static int hoveredX = 0;
      static int hoveredY = 0;
      static int hoveredNum = 0;
      switch (highlightMode) {
        case HighlightMode_Blocks:
          highlighted = (2 < x && x < 6) ^ (2 < y && y < 6);
          break;
        case HighlightMode_RelatedCells:
          highlighted = (x == hoveredX) || (y == hoveredY) ||
                        (x / 3 == hoveredX / 3 && y / 3 == hoveredY / 3);
          break;
        case HighlightMode_SameNumbers:
          highlighted = hoveredNum && sudoku.board[y][x] == hoveredNum;
      }

      if (x != 0) ImGui::SameLine();

      ImGui::PushID(x + y * 9);
      if (disabled) ImGui::PushStyleTextDisabled();
      if (ImGui::Selectable(sudoku.board[y][x]
                                ? std::to_string(sudoku.board[y][x]).c_str()
                                : "",
                            highlighted, 0, ImVec2(CellSize, CellSize)))
        if (!disabled && !sudoku.endTime) sudoku.board[y][x] = num;
      if (ImGui::IsItemHovered()) {
        hoveredX = x;
        hoveredY = y;
        hoveredNum = sudoku.board[y][x];
      }
      if (disabled) ImGui::PopStyleColor();
      ImGui::PopID();
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();

  // Numpad
  ImGui::BeginChild(
      "SudokuChildR",
      {GetRealWidth(3 * 25, 3, true), GetRealHeight(4 * CellSize, 4, true)},
      true);
  Game::NumPadWindow(&num);
  ImGui::EndChild();

  if (sudoku.endTime) {
    if (sudoku.errtype[0] == 'b') {  // errtype == "best"
      ImGui::PushStyleButtonColored(1, 7);
      ImGui::Button("Best!");
    } else {
      ImGui::PushStyleButtonColored(2, 7);
      ImGui::Button("Clear");
    }
    ImGui::PopStyleButtonColored();
    ImGui::SameLine();
    ImGui::TimeText(sudoku.endTime - sudoku.startTime);
  } else {
    if (ImGui::Button("Check")) sudoku.check();
    ImGui::SameLine();
    ImGui::TimeText((float)ImGui::GetTime() - sudoku.startTime);
  }
  if (sudoku.bestTime) {
    ImGui::SameLine();
    ImGui::Text("/");
    ImGui::SameLine();
    ImGui::TimeText(sudoku.bestTime);
  }

  if (sudoku.err) {
    ImGui::SameLine();
    ImGui::ErrorText(" <Error in %s %d>", sudoku.errtype, sudoku.erridx + 1);
  }

  // Level details
  if (ImGui::Difficulty(sudoku.difficulty)) showSelectLevel = true;
  ImGui::SameLine();
  ImGui::Text("Author: %s", sudoku.author.c_str());
  ImGui::PopStyleVar();
  ImGui::End();
}

static int neighbors[9][2]{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                           {0, 1},   {1, -1}, {1, 0},  {1, 1}};

static int blocks[9][2]{
    {1, 1}, {1, 4}, {1, 7}, {4, 1}, {4, 4}, {4, 7}, {7, 1}, {7, 4}, {7, 7},
};

/**
 * @brief 重置游戏并进入指定关卡.
 * @param id 关卡 id
 */
Sudoku::Sudoku(const char *id)
    : id{id},
      startTime{(float)ImGui::GetTime()},
      endTime{0.0f},
      err{false},
      errtype{"row"},
      erridx{0} {
  auto node = YAML::LoadFile(std::string("../levels/sudoku/") + id + ".yaml");
  this->author = node["author"].as<std::string>();
  this->difficulty = node["difficulty"].as<int>();
  this->bestTime = Save::getBestScore("sudoku", id);

  for (int y = 0; y < 9; y++)
    for (int x = 0; x < 9; x++)
      this->clues[y][x] = this->board[y][x] = node["question"][y][x].as<int>();
}

void Sudoku::clear() {
  memcpy(this->board[0], this->clues[0], sizeof(this->clues));
}

void Sudoku::check() {
  int cols[9]{0};
  for (int y = 0; y < 9; y++) {
    int row = 0;
    for (int x = 0; x < 9; x++) {
      row |= 1 << board[y][x];
      cols[x] |= 1 << board[y][x];
    }
    if (row != 0x3fe) {
      strcpy(errtype, "row");
      erridx = y;
      err = true;
      return;
    }
  }

  for (int x = 0; x < 9; x++)
    if (cols[x] != 0x3fe) {
      strcpy(errtype, "column");
      erridx = x;
      err = true;
      return;
    }

  for (int i = 0; i < 9; i++) {
    int block = 0;
    for (int j = 0; j < 9; j++)
      block |= 1 << board[blocks[i][0] + neighbors[j][0]]
                         [blocks[i][1] + neighbors[j][1]];
    if (block != 0x3fe) {
      strcpy(errtype, "block");
      erridx = i;
      err = true;
      return;
    }
  }

  errtype[0] = '\0';
  err = false;
  endTime = (float)ImGui::GetTime();
  if (!bestTime || (endTime - startTime <= bestTime)) {
    strcpy(errtype, "best");
    Save::setBestScore("sudoku", id.c_str(), bestTime = endTime - startTime);
  }
  return;
}
