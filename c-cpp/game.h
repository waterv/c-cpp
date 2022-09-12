#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

const int CellSize = 25;
const int DifficultySize = 60;

namespace ImGui {
// misc.cpp
void PushStyleTextDisabled();
void PushStyleTextPlotHistogram();
void PushStyleButtonColored(int idx, int max);
void PopStyleButtonColored();

void TimeText(int time);
void TimeText(float time);
void ErrorText(const char *fmt, ...);
bool Difficulty(int difficulty);

void NumPadWindow(int *p_num);
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

void SudokuWindow(bool *p_open);  // sudoku.cpp
}  // namespace Game

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

  Puzzle(const char *game, const char *id)
      : game{game},
        id{id},
        history{},
        historyIndex{-1},
        startTime{(float)ImGui::GetTime()},
        endTime{0.0f},
        err{false} {
    node =
        YAML::LoadFile(std::string("../levels/") + game + "/" + id + ".yaml");
    author = node["author"].as<std::string>();
    difficulty = node["difficulty"].as<int>();
    bestTime = Game::getBestTime(game, id);
  };

  void TutorialWindow(bool *p_open) const {
    Game::TutorialWindow(game.c_str(), p_open);
  };
  bool LevelSelectWindow(bool *p_open, std::string *p_level) const {
    return Game::LevelSelectWindow(game.c_str(), p_open, p_level);
  };

  bool canUndo() const { return historyIndex >= 0; };
  bool canRedo() const { return historyIndex < (int)history.size() - 1; }
  virtual void undo_(Operation op) { return; };
  virtual void redo_(Operation op) { return; };
  void undo() {
    if (!canUndo()) return;
    undo_(history[historyIndex]);
    historyIndex -= 1;
  };
  void redo() {
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
    history.clear();
    historyIndex = -1;
    clear_();
  }
};

#endif
