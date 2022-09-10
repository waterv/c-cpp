#ifndef __SUDOKU_H__
#define __SUDOKU_H__

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

void SudokuTutorialWindow(bool *p_open);

#endif
