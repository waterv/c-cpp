#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

namespace Game {
// game.cpp
void NumPadWindow(int *p_num);
void TutorialWindow(const char *name, bool *p_open);
bool LevelSelectWindow(const char *name, bool *p_open, std::string *p_level);
// sudoku.cpp
void SudokuWindow(bool *p_open);
}  // namespace Game

#endif
