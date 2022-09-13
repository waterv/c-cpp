#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

const int CellSize = 25;
const int DifficultySize = 60;

namespace ImGui {
// misc.cpp
void PushStyleTextDisabled();
void PushStyleTextPlotHistogram();
void PushStyleButtonColored(int idx, int max);
void PopStyleButtonColored();

ImU32 BorderColor();
ImU32 BackgroundColor(float alpha = 0.3f);
ImVec4 GetPos(ImVec2 screenPos, int x, int y);

void DrawLine(ImDrawList *drawList, ImVec4 pos, ImU32 col = BorderColor(),
              float thickness = 1.0f);
void DrawRectFilled(ImDrawList *drawList, ImVec4 pos,
                    ImU32 col = BackgroundColor());
void DrawBorder(ImDrawList *drawList, ImVec2 screenPos, int x, int y,
                ImGuiDir dir, ImU32 col = BorderColor(),
                float thickness = 1.0f);

void TimeText(int time);
void TimeText(float time);
bool Difficulty(int difficulty);

void NumPadChild(const char *id, int *p_num);
}  // namespace ImGui

float GetRealWidth(float width, int itemCount, bool hasPadding = false);
float GetRealHeight(float height, int itemCount, bool hasPadding = false);
std::vector<std::string> split(std::string str, char delim);

namespace Game {
// game.cpp
void TutorialWindow(const char *game, bool *p_open);
bool LevelSelectWindow(const char *game, bool *p_open, std::string *p_level);

// save.cpp
float getBestTime(const char *game, const char *id);
void setBestTime(const char *game, const char *id, float time);
bool getLevelSelectData(const char *game, std::string *tid, int *page);
void setLevelSelectData(const char *game, std::string tid, int page);

void SudokuWindow(bool *p_open);  // cpp
void KakuroWindow(bool *p_open);  // kakuro.cpp
}  // namespace Game

struct NumberOperation {
  int x, y, prev, curr;
};

template <class Operation>
struct Puzzle {
  YAML::Node node;
  std::string game;
  std::string id;
  std::string author;
  int difficulty;

  std::vector<Operation> history;
  int historyIndex;

  float startTime;
  float endTime;
  float bestTime;

  bool err;
  std::string errType;

  bool showTutorial;
  bool showLevelSelect;

  Puzzle(const char *game, const char *id)
      : game{game},
        id{id},
        history{},
        historyIndex{-1},
        startTime{(float)ImGui::GetTime()},
        endTime{0.0f},
        err{false},
        errType{""},
        showTutorial{false},
        showLevelSelect{false} {
    node =
        YAML::LoadFile(std::string("../levels/") + game + "/" + id + ".yaml");
    author = node["author"].as<std::string>();
    difficulty = node["difficulty"].as<int>();
    bestTime = Game::getBestTime(game, id);
  };

  void TutorialWindow() {
    if (showTutorial) Game::TutorialWindow(game.c_str(), &showTutorial);
  };

  bool LevelSelectWindow(std::string *p_id) {
    if (!showLevelSelect) return false;
    return Game::LevelSelectWindow(game.c_str(), &showLevelSelect, p_id);
  };

  void LevelDetails(const char *fmt, ...) {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
      if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        if (ImGui::IsKeyReleased(ImGuiKey_O)) showLevelSelect = true;
        if (ImGui::IsKeyReleased(ImGuiKey_Z) && canUndo()) undo();
        if (ImGui::IsKeyReleased(ImGuiKey_Y) && canRedo()) redo();
      }

    if (endTime) {
      if (errType == "best") {
        ImGui::PushStyleButtonColored(1, 7);
        ImGui::Button("Best!");
      } else {
        ImGui::PushStyleButtonColored(2, 7);
        ImGui::Button("Clear!");
      }
      ImGui::PopStyleButtonColored();
      ImGui::SameLine();
      ImGui::TimeText(endTime - startTime);
    } else {
      if (ImGui::Button("Check")) check();
      ImGui::SameLine();
      ImGui::TimeText((float)ImGui::GetTime() - startTime);
    }

    if (bestTime) {
      ImGui::SameLine();
      ImGui::Text("/");
      ImGui::SameLine();
      ImGui::TimeText(bestTime);
    }

    if (err) {
      ImGui::SameLine();
      va_list args;
      va_start(args, fmt);
      ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.0f, 0.0f, 1.0f});
      ImGui::TextV(fmt, args);
      ImGui::PopStyleColor();
      va_end(args);
    }

    if (ImGui::Difficulty(difficulty)) showLevelSelect = true;
    ImGui::SameLine();
    ImGui::Text("Author: %s", author.c_str());
  }

  bool canUndo() const { return historyIndex >= 0; };
  bool canRedo() const { return historyIndex < (int)history.size() - 1; }
  virtual void undo_(Operation op) { return; };
  virtual void redo_(Operation op) { return; };
  void undo() {
    if (endTime) return;
    if (!canUndo()) return;
    undo_(history[historyIndex]);
    historyIndex -= 1;
  };
  void redo() {
    if (endTime) return;
    if (!canRedo()) return;
    historyIndex += 1;
    redo_(history[historyIndex]);
  };

  void pushHistory(Operation op) {
    historyIndex += 1;
    if (historyIndex < (int)history.size())
      history[historyIndex] = op;
    else
      history.push_back(op);
  }

  virtual void clear_() { return; };
  void clear() {
    if (endTime) return;
    history.clear();
    historyIndex = -1;
    clear_();
  }

  virtual void check() { return; };
  void win() {
    errType = "";
    err = false;
    endTime = (float)ImGui::GetTime();
    if (!bestTime || (endTime - startTime <= bestTime)) {
      errType = "best";
      Game::setBestTime(game.c_str(), id.c_str(),
                        bestTime = endTime - startTime);
    }
  }
};

#endif
