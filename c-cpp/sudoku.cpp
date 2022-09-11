#include "game.h"

class Sudoku {
 public:
  YAML::Node node;

  int question[9][9];
  int board[9][9];

  float startTime;
  float endTime;

  bool err;
  char errtype[7];
  int erridx;

  Sudoku(const char *filename);
  Sudoku(Sudoku &sudoku);
  void clear();
  void check();
};

void Game::SudokuWindow(bool *p_open) {
  static Sudoku sudoku{"../levels/sudoku/psv-1.yaml"};

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

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      showSelectLevel = ImGui::MenuItem("Select Level..");
      if (ImGui::MenuItem("Clear")) sudoku.clear();
      if (ImGui::MenuItem("Reset")) sudoku = Sudoku{sudoku};
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
      sudoku = Sudoku{
          (std::string("../levels/sudoku/") + levelSelected + ".yaml").c_str()};

  if (showTutorialWindow) Game::TutorialWindow("sudoku", &showTutorialWindow);

  // Select number by keyboard
  static int num = 1;
  for (ImGuiKey key = ImGuiKey_0; key <= ImGuiKey_9; key++)
    if (ImGui::IsKeyDown(key)) num = key - ImGuiKey_0;

  // Board
  ImGui::BeginChild("SudokuChildL",
                    {GetRealWidth(9 * CellSize, 9, true),
                     GetRealHeight(9 * CellSize, 9, true)},
                    true);
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      bool disabled = (bool)sudoku.question[y][x];
      bool selected = (2 < x && x < 6) ^ (2 < y && y < 6);

      if (x != 0) ImGui::SameLine();

      ImGui::PushID(x + y * 9);
      if (disabled) ImGui::PushStyleTextDisabled();
      if (ImGui::Selectable(sudoku.board[y][x]
                                ? std::to_string(sudoku.board[y][x]).c_str()
                                : "",
                            selected, 0, ImVec2(CellSize, CellSize)))
        if (!disabled && !sudoku.endTime) sudoku.board[y][x] = num;
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
    ImGui::PushStyleButtonColored(2, 7);
    ImGui::Button("Clear");
    ImGui::PopStyleButtonColored();
    ImGui::SameLine();
    ImGui::TimeText(sudoku.endTime - sudoku.startTime);
  } else {
    if (ImGui::Button("Check")) sudoku.check();
    ImGui::SameLine();
    ImGui::TimeText((float)ImGui::GetTime() - sudoku.startTime);
  }
  if (sudoku.err) {
    ImGui::SameLine();
    ImGui::ErrorText(" <Error in %s %d>", sudoku.errtype, sudoku.erridx + 1);
  }

  // Level details
  ImGui::Difficulty(sudoku.node["difficulty"].as<int>());
  ImGui::SameLine();
  ImGui::Text("Author: %s", sudoku.node["author"].as<std::string>().c_str());

  ImGui::PopStyleVar();
  ImGui::End();
}

static int neighbors[9][2]{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                           {0, 1},   {1, -1}, {1, 0},  {1, 1}};

static int blocks[9][2]{
    {1, 1}, {1, 4}, {1, 7}, {4, 1}, {4, 4}, {4, 7}, {7, 1}, {7, 4}, {7, 7},
};

Sudoku::Sudoku(const char *filename)
    : startTime{(float)ImGui::GetTime()},
      endTime{0.0f},
      err{false},
      errtype{"row"},
      erridx{0} {
  if (filename) this->node = YAML::LoadFile(filename);
  for (int y = 0; y < 9; y++)
    for (int x = 0; x < 9; x++)
      this->question[y][x] = this->board[y][x] =
          this->node["question"][y][x].as<int>();
}

Sudoku::Sudoku(Sudoku &sudoku)
    : node{sudoku.node},
      startTime{(float)ImGui::GetTime()},
      endTime{0.0f},
      err{false},
      errtype{"row"},
      erridx{0} {
  memcpy(this->board[0], sudoku.question[0], sizeof(sudoku.question));
  memcpy(this->question[0], sudoku.question[0], sizeof(sudoku.question));
}

void Sudoku::clear() {
  memcpy(this->board[0], this->question[0], sizeof(this->question));
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

  err = false;
  endTime = (float)ImGui::GetTime();
  return;
}
