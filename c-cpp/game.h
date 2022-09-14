#ifndef __GAME_H__
#define __GAME_H__

#include "main.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

void SudokuWindow(bool *p_open);
void KakuroWindow(bool *p_open);
void AkariWindow(bool* p_open);

// 棋盘格子的大小.
const int CellSize = 25;

namespace ImGui {

/**
 * @brief 将 ImGuiCol_Text 样式设置为 ImGuiCol_TextDisabled.
 */
void PushStyleTextDisabled();

/**
 * @brief 将 ImGuiCol_Text 样式设置为 ImGuiCol_PlotHistogram.
 */
void PushStyleTextPlotHistogram();

/**
 * @brief 将 ImGuiCol_Button 样式设置为色相为 idx/max 的系列颜色.
 * 该样式需使用 ImGui::PopStyleButtonColored() 函数结束.
 */
void PushStyleButtonColored(int idx, int max);
void PopStyleButtonColored();

ImU32 BorderColor();
ImU32 BackgroundColor(float alpha = 0.3f);

/**
 * @brief 取棋盘上某格的左上右下坐标.
 *
 * @param screenPos 使用 ImGui::GetCursorScreenPos() 函数获取.
 * @param x 该格是一行中从左到右第几个, 从 1 开始.
 * @param y 该格是一列中从上到下第几个, 从 1 开始.
 * @return 左上 (x, y), 右下 (z, w).
 */
ImVec4 GetPos(ImVec2 screenPos, int x, int y);

void DrawLine(ImDrawList *drawList, ImVec4 pos, ImU32 col = BorderColor(),
              float thickness = 1.0f);
void DrawRectFilled(ImDrawList *drawList, ImVec4 pos,
                    ImU32 col = BackgroundColor());

/**
 * @brief 为棋盘上某格绘制某方向上的边框.
 *
 * @param drawList 使用 ImGui::GetWindowDrawList() 函数获取.
 * @param screenPos 使用 ImGui::GetCursorScreenPos() 函数获取.
 * @param x 该格是一行中从左到右第几个, 从 1 开始.
 * @param y 该格是一列中从上到下第几个, 从 1 开始.
 * @param dir ImGuiDir_{Left,Right,Up,Down}
 */
void DrawBorder(ImDrawList *drawList, ImVec2 screenPos, int x, int y,
                ImGuiDir dir, ImU32 col = BorderColor(),
                float thickness = 1.0f);

/**
 * @brief 数字盘子窗口.
 *
 * @param id 子窗口的 ID.
 * @param p_num 当前选中数字的指针.
 */
void NumPadChild(const char *id, int *p_num);

}  // namespace ImGui

/**
 * @brief 获取若干个相同宽度的组件所占据的,
 * 考虑了组件间距和内补的实际宽度.
 *
 * @param width 单个组件的宽度.
 * @param itemCount 组件数目.
 * @param hasPadding 欲获取的宽度是否用于有边框子窗口.
 */
float GetRealWidth(float width, int itemCount, bool hasPadding = false);

/**
 * @brief 获取若干个相同高度的组件所占据的,
 * 考虑了组件间距和内补的实际高度.
 *
 * @param width 单个组件的高度.
 * @param itemCount 组件数目.
 * @param hasPadding 欲获取的高度是否用于有边框子窗口.
 */
float GetRealHeight(float height, int itemCount, bool hasPadding = false);

/**
 * @brief 使用给定分割符将字符串切割为若干子串.
 *
 * @param str 欲切割字符串.
 * @param delim 分割符.
 */
std::vector<std::string> split(std::string str, char delim);

template <class Operation>
struct Puzzle {
  YAML::Node node;
  std::string game, id, author;
  int difficulty;

  std::vector<Operation> history;
  int historyIndex;

  float startTime, endTime, bestTime;

  bool err;
  std::string errType;

  bool showTutorial, showLevelSelect;

  Puzzle(const char *game, const char *id);

  void TutorialWindow();
  bool LevelSelectWindow(std::string *p_id);
  void LevelDetails(const char *fmt, ...);

  void pushHistory(Operation op);
  bool canUndo() const { return historyIndex >= 0; };
  bool canRedo() const { return historyIndex < (int)history.size() - 1; }
  void undo();
  void redo();
  void clear();
  void win();

  virtual void undo_(Operation op) { return; };
  virtual void redo_(Operation op) { return; };
  virtual void clear_() { return; };
  virtual void check() { return; };
};

struct NumberOperation {
  int x, y, prev, curr;
};

#include "internal.h"

#endif
