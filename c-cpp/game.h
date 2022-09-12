#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

namespace Game {
// game.cpp
void NumPadWindow(int *p_num);
void TutorialWindow(const char *game, bool *p_open);
bool LevelSelectWindow(const char *game, bool *p_open, std::string *p_level);
// sudoku.cpp
void SudokuWindow(bool *p_open);
}  // namespace Game

namespace Save {
float getBestScore(const char *game, const char *id);
void setBestScore(const char *game, const char *id, float time);
bool getLevelSelectData(const char *game, std::string *tid, int *page);
void setLevelSelectData(const char *game, std::string tid, int page);
}  // namespace Save

#endif
