#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include "game.h"

namespace Internal {
void TimeText(int time);
void TimeText(float time);
bool Difficulty(int difficulty);

void TutorialWindow(const char *game, bool *p_open);
bool LevelSelectWindow(const char *game, bool *p_open, std::string *p_level);

float getBestTime(const char *game, const char *id);
void setBestTime(const char *game, const char *id, float time);
bool getLevelSelectData(const char *game, std::string *tid, int *page);
void setLevelSelectData(const char *game, std::string tid, int page);
}  // namespace Internal

template <class Operation>
Puzzle<Operation>::Puzzle(const char *game, const char *id)
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
  node = YAML::LoadFile(std::string("../levels/") + game + "/" + id + ".yaml");
  author = node["author"].as<std::string>();
  difficulty = node["difficulty"].as<int>();
  bestTime = Internal::getBestTime(game, id);
}

template <class Operation>
void Puzzle<Operation>::TutorialWindow() {
  if (showTutorial) Internal::TutorialWindow(game.c_str(), &showTutorial);
}

template <class Operation>
bool Puzzle<Operation>::LevelSelectWindow(std::string *p_id) {
  if (!showLevelSelect) return false;
  return Internal::LevelSelectWindow(game.c_str(), &showLevelSelect, p_id);
}

template <class Operation>
void Puzzle<Operation>::LevelDetails(const char *fmt, ...) {
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
    Internal::TimeText(endTime - startTime);
  } else {
    if (ImGui::Button("Check")) check();
    ImGui::SameLine();
    Internal::TimeText((float)ImGui::GetTime() - startTime);
  }

  if (bestTime) {
    ImGui::SameLine();
    ImGui::Text("/");
    ImGui::SameLine();
    Internal::TimeText(bestTime);
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

  if (Internal::Difficulty(difficulty)) showLevelSelect = true;
  ImGui::SameLine();
  ImGui::Text("Author: %s", author.c_str());
}

template <class Operation>
void Puzzle<Operation>::undo() {
  if (endTime) return;
  if (!canUndo()) return;
  undo_(history[historyIndex]);
  historyIndex -= 1;
}

template <class Operation>
void Puzzle<Operation>::redo() {
  if (endTime) return;
  if (!canRedo()) return;
  historyIndex += 1;
  redo_(history[historyIndex]);
}

template <class Operation>
void Puzzle<Operation>::pushHistory(Operation op) {
  historyIndex += 1;
  if (historyIndex < (int)history.size())
    history[historyIndex] = op;
  else
    history.push_back(op);
}

template <class Operation>
void Puzzle<Operation>::clear() {
  if (endTime) return;
  history.clear();
  historyIndex = -1;
  clear_();
}

template <class Operation>
void Puzzle<Operation>::win() {
  errType = "";
  err = false;
  endTime = (float)ImGui::GetTime();
  if (!bestTime || (endTime - startTime <= bestTime)) {
    errType = "best";
    Internal::setBestTime(game.c_str(), id.c_str(),
                          bestTime = endTime - startTime);
  }
}

#endif
