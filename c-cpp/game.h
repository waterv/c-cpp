#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

const char numStr[10][3]{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

namespace Game {
void NumPadWindow(int *p_num);
void SudokuWindow(bool *p_open);
}  // namespace Game

#endif
