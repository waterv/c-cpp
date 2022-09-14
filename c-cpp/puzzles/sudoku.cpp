#include "game.h"

// 记录游戏中的一次操作, 用于撤销 / 重做功能.
using SudokuOperation = NumberOperation;

// 游戏类继承自抽象模板类 Puzzle, 模板参数为操作记录类型.
struct Sudoku : public Puzzle<SudokuOperation> {
  // 题目内容.
  int clues[9][9];

  // 当前盘面. 值为 0 表示空格, 1-9 表示填入数字, 11-19 表示标记数字.
  int board[9][9];

  // 错误类型或新纪录标记, 取值有 "row", "column", "block", "best".
  // std::string errType;

  // 错误出现的位置.
  int errIndex;

  // 构造函数, 效果是重置游戏并进入指定关卡.
  // 关卡 id 形如 "psv/1", 即 ../levels/<Game Name>/ 起的相对路径.
  Sudoku(const char *id);

  // 游戏操作的统一接口.
  void input(int x, int y, int num);

  // 实现撤销 / 重做 / 清空功能需要重写这三个虚函数.
  // 调用时删去函数名末尾的 _.
  void undo_(SudokuOperation op) { board[op.y][op.x] = op.prev; };
  void redo_(SudokuOperation op) { board[op.y][op.x] = op.curr; };
  void clear_() { memcpy(board[0], clues[0], sizeof(board)); };

  // 检查是否正解.
  // 若是则更新 err, endTime, 可能更新 bestTime, errType;
  // 若否则更新 err, errType, errIndex.
  void check();
};

enum HighlightMode {
  HighlightMode_Blocks,
  HighlightMode_RelatedCells,
  HighlightMode_SameNumbers,
  HighlightModes,
};

void SudokuWindow(bool *p_open) {
  static Sudoku sudoku{"psv/1"};

  if (!ImGui::Begin("Sudoku", p_open,
                    ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_MenuBar))
    return ImGui::End();  // 如果窗口被折叠, 则直接短路不计算窗口内容.

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});

  // 窗口顶部的菜单栏.
  static bool showBorder = true;
  static int highlightMode = HighlightMode_Blocks;
  bool highlightModeBools[HighlightModes];
  for (int i = 0; i < HighlightModes; i++)
    highlightModeBools[i] = highlightMode == i;

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      if (ImGui::MenuItem("Select Level..", "Ctrl+O"))
        sudoku.showLevelSelect = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Undo", "Ctrl+Z", false, sudoku.canUndo()))
        sudoku.undo();
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false, sudoku.canRedo()))
        sudoku.redo();
      ImGui::Separator();
      if (ImGui::MenuItem("Clear")) sudoku.clear();
      if (ImGui::MenuItem("Reset")) sudoku = Sudoku{sudoku.id.c_str()};
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Show Border", NULL, showBorder))
        showBorder = !showBorder;
      if (ImGui::BeginMenu("Highlight Mode")) {
        if (ImGui::MenuItem("3x3 Blocks", "Ctrl+1", highlightModeBools))
          highlightMode = HighlightMode_Blocks;
        if (ImGui::MenuItem("Related Cells", "Ctrl+2", highlightModeBools + 1))
          highlightMode = HighlightMode_RelatedCells;
        if (ImGui::MenuItem("Same Numbers", "Ctrl+3", highlightModeBools + 2))
          highlightMode = HighlightMode_SameNumbers;
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("Tutorial")) sudoku.showTutorial = true;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // 选择关卡和教程.
  // [WARNING] 如果棋盘大小不固定, 务必重置函数中与数组下标有关的 static 变量.
  static std::string id;
  if (sudoku.LevelSelectWindow(&id)) sudoku = Sudoku{id.c_str()};
  sudoku.TutorialWindow();

  // 选择数字和高亮模式的键鼠快捷键.
  // Ctrl+O/Z/Y 三个通用快捷键被 LevelDetails 方法接管.
  static int num = 1;
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
      for (ImGuiKey key = ImGuiKey_1; key <= ImGuiKey_3; key++)
        if (ImGui::IsKeyDown(key)) highlightMode = key - ImGuiKey_1;
    } else {
      for (ImGuiKey key = ImGuiKey_0; key <= ImGuiKey_9; key++)
        if (ImGui::IsKeyDown(key)) num = key - ImGuiKey_0;
    }

    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel > 0)
      num = (num + 10 - 1) % 10;
    else if (wheel < 0)
      num = (num + 1) % 10;
  }

  // 左侧的盘面.
  ImGui::BeginChild(
      "SudokuChildL",
      {GetRealWidth(CellSize, 9, true), GetRealHeight(CellSize, 9, true)},
      true);
  const auto pos = ImGui::GetCursorScreenPos();
  const auto col = ImGui::BorderColor();
  const auto bac = ImGui::BackgroundColor();
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  for (int y = -1; y < 9; y++) {
    for (int x = -1; x < 9; x++) {
      if (showBorder) {
        if (y != -1)
          ImGui::DrawBorder(drawList, pos, x + 1, y + 1, ImGuiDir_Right,
                            (x + 1) % 3 ? bac : col, (x + 1) % 3 ? 1.0f : 3.0f);
        if (x != -1)
          ImGui::DrawBorder(drawList, pos, x + 1, y + 1, ImGuiDir_Down,
                            (y + 1) % 3 ? bac : col, (y + 1) % 3 ? 1.0f : 3.0f);
      }
      if (x == -1 || y == -1) continue;

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
      bool disabled = sudoku.clues[y][x] != 0;
      bool isNote = sudoku.board[y][x] > 9;
      if (disabled) ImGui::PushStyleTextDisabled();
      if (isNote) ImGui::PushStyleTextPlotHistogram();
      if (ImGui::Selectable(
              sudoku.board[y][x] % 10
                  ? std::to_string(sudoku.board[y][x] % 10).c_str()
                  : "",
              highlighted, 0, ImVec2(CellSize, CellSize)))
        sudoku.input(x, y, num);
      if (ImGui::IsItemHovered()) {
        hoveredX = x;
        hoveredY = y;
        hoveredNum = sudoku.board[y][x];
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) ||
            ImGui::IsKeyReleased(ImGuiKey_Minus)) {
          sudoku.input(x, y, num + 10);
        } else if (ImGui::IsKeyReleased(ImGuiKey_Backspace)) {
          sudoku.input(x, y, 0);
        }
      }
      if (isNote) ImGui::PopStyleColor();
      if (disabled) ImGui::PopStyleColor();
      ImGui::PopID();
    }
  }
  ImGui::EndChild();

  // 右侧的数字盘.
  ImGui::SameLine();
  ImGui::NumPadChild("SudokuChildR", &num);

  // 下方的提交、用时、关卡详情等信息, 同时接管了 Ctrl+O/Z/Y 快捷键.
  sudoku.LevelDetails(" <Error in %s %d>", sudoku.errType.c_str(),
                      sudoku.errIndex + 1);
  ImGui::PopStyleVar();
  ImGui::End();
}

static int neighbors[9][2]{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0},
                           {0, 1},   {1, -1}, {1, 0},  {1, 1}};

static int blocks[9][2]{
    {1, 1}, {1, 4}, {1, 7}, {4, 1}, {4, 4}, {4, 7}, {7, 1}, {7, 4}, {7, 7},
};

Sudoku::Sudoku(const char *id) : Puzzle{"sudoku", id}, errIndex{0} {
  for (int y = 0; y < 9; y++)
    for (int x = 0; x < 9; x++)
      clues[y][x] = board[y][x] = node["question"][y][x].as<int>();
}

void Sudoku::input(int x, int y, int num) {
  if (board[y][x] == num) return;
  if (clues[y][x] != 0) return;
  if (endTime) return;
  pushHistory({x, y, board[y][x], num});
  board[y][x] = num;
}

void Sudoku::check() {
  err = true;
  int cols[9]{0};
  for (int y = 0; y < 9; y++) {
    int row = 0;
    for (int x = 0; x < 9; x++) {
      row |= 1 << board[y][x];
      cols[x] |= 1 << board[y][x];
    }
    if (row != 0x3fe) {
      errType = "row";
      errIndex = y;
      return;
    }
  }

  for (int x = 0; x < 9; x++)
    if (cols[x] != 0x3fe) {
      errType = "column";
      errIndex = x;
      return;
    }

  for (int i = 0; i < 9; i++) {
    int block = 0;
    for (int j = 0; j < 9; j++)
      block |= 1 << board[blocks[i][0] + neighbors[j][0]]
                         [blocks[i][1] + neighbors[j][1]];
    if (block != 0x3fe) {
      errType = "block";
      errIndex = i;
      return;
    }
  }

  return win();
}
