#include "game.h"

using SlitherlinkOperation = NumberOperation;

struct Slitherlink : public Puzzle<NumberOperation> {
  int width, height;
  std::vector<std::vector<int>> clues, board;

  // errType = "wrong number of lines", "line crossing", "multiple loop"
  int errX, errY;

  Slitherlink(const char* id);

  void input(int x, int y, ImVec2 d, bool isNote);
  void input(int x, int y, ImGuiDir dir, bool isNote);

  void undo_(SlitherlinkOperation op) { board[op.y][op.x] = op.prev; };
  void redo_(SlitherlinkOperation op) { board[op.y][op.x] = op.curr; };
  void clear_() {
    for (auto& u : board)
      for (auto& v : u) v = 0;
  };

  bool hasLine(int x, int y, ImGuiDir dir);
  int pointLineCount(int x, int y);
  int cellLineCount(int x, int y);
  void check();
};

void SlitherlinkWindow(bool* p_open) {
  static Slitherlink slither{"psv12/1"};

  if (!ImGui::Begin("Slitherlink", p_open,
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
        slither.showLevelSelect = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Undo", "Ctrl+Z", false, slither.canUndo()))
        slither.undo();
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false, slither.canRedo()))
        slither.redo();
      ImGui::Separator();
      if (ImGui::MenuItem("Clear")) slither.clear();
      if (ImGui::MenuItem("Reset")) slither = Slitherlink{slither.id.c_str()};
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Show Border", NULL, showBorder))
        showBorder = !showBorder;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("Tutorial")) slither.showTutorial = true;
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  static std::string id;
  if (slither.LevelSelectWindow(&id)) slither = Slitherlink{id.c_str()};
  slither.TutorialWindow();

  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
    ;

  ImGui::BeginChild("SlitherChild",
                    {GetRealWidth(CellSize, slither.width, true),
                     GetRealHeight(CellSize, slither.height, true)},
                    true);
  const auto pos = ImGui::GetCursorScreenPos();
  const auto col = ImGui::BorderColor();
  const auto bac = ImGui::BackgroundColor();
  const auto note = ImGui::NoteColor();
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  for (int y = 0; y <= slither.height; y++)
    for (int x = 0; x <= slither.width; x++) {
      if (showBorder) {
        if (y) ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right, bac);
        if (x) ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down, bac);
      }

      if (slither.hasLine(x, y, ImGuiDir_Right))
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right, col, 3.0f);
      if (slither.hasLine(x, y, ImGuiDir_Down))
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down, col, 3.0f);

      if (slither.board[y][x] & (1 << ImGuiDir_Left))
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Right, note);
      if (slither.board[y][x] & (1 << ImGuiDir_Up))
        ImGui::DrawBorder(drawList, pos, x, y, ImGuiDir_Down, note);

      if (!x || !y) continue;
      if (x != 1) ImGui::SameLine();

      ImGui::PushID(x + y * slither.width);
      ImGui::Selectable(
          slither.clues[y][x] ? std::to_string(slither.clues[y][x] - 1).c_str()
                              : "",
          slither.board[y][x] & (1 << ImGuiDir_COUNT), 0, {CellSize, CellSize});

      static ImVec2 drag = ImGui::GetMouseDragDelta();
      if (ImGui::IsItemActive()) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
          ImGui::GetForegroundDrawList()->AddLine(
              ImGui::GetIO().MouseClickedPos[0], ImGui::GetIO().MousePos,
              ImGui::GetColorU32(ImGuiCol_Button), 2.0f);
          drag = ImGui::GetMouseDragDelta();
        }
        for (int i = 0; i < 4; i++) {
          if (ImGui::IsKeyDown(ImGuiKey_LeftArrow + i))
            slither.input(x, y, ImGuiDir_Left + i,
                          ImGui::IsKeyDown(ImGuiKey_LeftShift));
        }
      }
      if (ImGui::IsItemDeactivated() &&
          ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        slither.input(x, y, drag, ImGui::IsMouseDown(ImGuiMouseButton_Right));

      ImGui::PopID();
    }
  ImGui::EndChild();

  slither.LevelDetails(" <Error: there is %s at (%d, %d)>",
                       slither.errType.c_str(), slither.errX, slither.errY);
  ImGui::PopStyleVar();
  ImGui::End();
}

Slitherlink::Slitherlink(const char* id)
    : Puzzle{"slitherlink", id}, errX{0}, errY{0} {
  width = node["width"].as<int>();
  height = node["height"].as<int>();
  for (int y = 0; y <= height; y++) {
    clues.push_back({0});
    board.push_back({0});
    for (int x = 1; x <= width; x++) {
      clues[y].push_back(y ? node["question"][y - 1][x - 1].as<int>() : 0);
      board[y].push_back(0);
    }
  }
}

void Slitherlink::input(int x, int y, ImVec2 drag, bool isNote) {
  if (!drag.x && !drag.y) return input(x, y, ImGuiDir_COUNT, true);
  if (ABS(drag.x) < ABS(drag.y))
    return input(x, y, (drag.y < 0) ? ImGuiDir_Up : ImGuiDir_Down, isNote);
  return input(x, y, (drag.x < 0) ? ImGuiDir_Left : ImGuiDir_Right, isNote);
}

void Slitherlink::input(int x, int y, ImGuiDir dir, bool isNote) {
  ImGuiDir dir_, nonDir;
  switch (dir) {
    case ImGuiDir_Left:
      return input(x - 1, y, ImGuiDir_Right, isNote);
    case ImGuiDir_Up:
      return input(x, y - 1, ImGuiDir_Down, isNote);
    case ImGuiDir_Right:
      dir_ = isNote ? ImGuiDir_Left : dir;
      nonDir = isNote ? dir : ImGuiDir_Left;
      break;
    case ImGuiDir_Down:
      dir_ = isNote ? ImGuiDir_Up : dir;
      nonDir = isNote ? dir : ImGuiDir_Up;
      break;
    case ImGuiDir_COUNT:
      dir_ = dir;
      nonDir = 0;
      break;
  }
  int curr = (board[y][x] ^ (1 << dir_)) & ~(1 << nonDir);
  pushHistory({x, y, board[y][x], curr});
  board[y][x] = curr;
}

bool Slitherlink::hasLine(int x, int y, ImGuiDir dir) {
  switch (dir) {
    case ImGuiDir_Left:
      return !!(board[y][x - 1] & (1 << ImGuiDir_Right));
    case ImGuiDir_Up:
      return !!(board[y - 1][x] & (1 << ImGuiDir_Down));
    default:
      return !!(board[y][x] & (1 << dir));
  }
}

int Slitherlink::pointLineCount(int x, int y) {
  int count =
      (int)hasLine(x, y, ImGuiDir_Right) + (int)hasLine(x, y, ImGuiDir_Down);
  if (x != width) count += (int)hasLine(x + 1, y, ImGuiDir_Down);
  if (y != height) count += (int)hasLine(x, y + 1, ImGuiDir_Right);
  return count;
}

int Slitherlink::cellLineCount(int x, int y) {
  int count = 0;
  for (int dir = ImGuiDir_Left; dir <= ImGuiDir_Down; dir++)
    count += (int)hasLine(x, y, dir);
  return count;
}

void Slitherlink::check() {
  err = true;
  bool zeroLines = true;
  for (int y = 0; y <= height; y++)
    for (int x = 0; x <= width; x++) {
      errX = x;
      errY = y;
      int count = pointLineCount(x, y);
      if (count > 0) zeroLines = false;
      if (count == 1) return void(errType = "dead-end line");
      if (count == 3) return void(errType = "branch line");
      if (count == 4) return void(errType = "crossing line");
      if (x == 0 || y == 0) continue;
      if (clues[y][x] && cellLineCount(x, y) != clues[y][x] - 1) {
        errType = "wrong number of lines";
        return;
      }
    }
  if (zeroLines) return void(errType = "no line");
  // Todo: Loop Check
}
