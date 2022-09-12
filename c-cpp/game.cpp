#include "game.h"

void Game::NumPadWindow(int *p_num) {
  int num = 0;
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
      if (x != 0) ImGui::SameLine();
      ImGui::PushID(++num);
      if (ImGui::Selectable(std::to_string(num).c_str(), *p_num == num, 0,
                            ImVec2(CellSize, CellSize)))
        *p_num = num;
      ImGui::PopID();
    }
  }
  if (ImGui::Selectable("Eraser", *p_num == 0, 0,
                        ImVec2(GetRealWidth(3 * CellSize, 3), CellSize)))
    *p_num = 0;
}

void Game::TutorialWindow(const char *game, bool *p_open) {
  ImGui::Begin((std::string("Tutorial###Tutorial of ") + game).c_str(), p_open,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_AlwaysAutoResize);
  auto node = YAML::LoadFile(std::string("../levels/") + game +
                             "/info.yaml")["tutorials"];
  if (ImGui::BeginTabBar((std::string("Tutorial Tabbar of ") + game).c_str(),
                         ImGuiTabBarFlags_None)) {
    for (auto u : node) {
      auto title = u.first.as<std::string>();
      if (ImGui::BeginTabItem(title.c_str())) {
        for (auto v : u.second) {
          auto text = v.as<std::string>();
          ImGui::BulletText(text.c_str());
        }
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

struct GameLevel {
  int index;
  int difficulty;
  std::string author;
  float bestTime;
};

struct GameCategory {
  std::string tid;
  std::string origin;
  std::vector<GameLevel> levels;
};

struct LevelSelectData {
  YAML::Node node;
  std::string popupId;
  std::vector<GameCategory> categories;
  std::string categoriesStr;
  std::string selectedTId;
  int selectedCategoryIndex;
  int selectedPage;

  LevelSelectData(const char *game);
  GameCategory &selectedCategory();
  void updateSaveData(const char *game);
};

LevelSelectData::LevelSelectData(const char *game) {
  node =
      YAML::LoadFile(std::string("../levels/") + game + "/info.yaml")["levels"];
  popupId = std::string("Level Select###Level Select of ") + game;

  bool hasSaveData =
      Save::getLevelSelectData(game, &selectedTId, &selectedPage);
  if (!hasSaveData) selectedCategoryIndex = 0;

  categories.clear();
  categoriesStr = "";
  for (size_t i = 0; i < node.size(); i++) {
    int n = node[i]["number"].as<int>();
    auto tid = node[i]["tid"].as<std::string>();
    if (hasSaveData && tid == selectedTId) selectedCategoryIndex = i;

    auto origin = node[i]["origin"].as<std::string>();
    categoriesStr += origin + '\0';

    std::vector<GameLevel> levels;
    for (int j = 1; j <= n; j++) {
      std::string id = tid + "/" + std::to_string(j);
      auto level =
          YAML::LoadFile(std::string("../levels/") + game + "/" + id + ".yaml");
      levels.push_back({j, level["difficulty"].as<int>(),
                        level["author"].as<std::string>(),
                        Save::getBestScore(game, id.c_str())});
    }

    categories.push_back({tid, origin, levels});
  }
}

GameCategory &LevelSelectData::selectedCategory() {
  return categories[selectedCategoryIndex];
}

void LevelSelectData::updateSaveData(const char *game) {
  Save::setLevelSelectData(game, selectedCategory().tid, selectedPage);
}

bool Game::LevelSelectWindow(const char *game, bool *p_open,
                             std::string *p_level) {
  bool selected = false;
  static LevelSelectData d{game};

  if (ImGui::BeginPopupModal(d.popupId.c_str(), p_open,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    std::string tid = d.selectedCategory().tid;
    if (ImGui::Combo("Category", &d.selectedCategoryIndex,
                     d.categoriesStr.c_str())) {
      d.selectedPage = 0;
      d.updateSaveData(game);
    }

    if (ImGui::BeginTable((std::string("Level Table of ") + game).c_str(), 4)) {
      ImGui::TableSetupColumn("Difficulty");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Author");
      ImGui::TableSetupColumn("Best");
      ImGui::TableHeadersRow();
      auto levels = d.selectedCategory().levels;
      for (int i = 0;
           (i < 10) && (10 * d.selectedPage + i < (int)levels.size()); i++) {
        int index = 10 * d.selectedPage + i;
        auto level = levels[index];
        ImGui::PushID(i);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Difficulty(level.difficulty)) {
          selected = true;
          *p_open = false;
          *p_level = tid + "/" + std::to_string(index + 1);
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::Text(std::to_string(index + 1).c_str());

        ImGui::TableSetColumnIndex(2);
        ImGui::Text(level.author.c_str());

        ImGui::TableSetColumnIndex(3);
        if (level.bestTime)
          ImGui::TimeText(level.bestTime);
        else
          ImGui::Text("-");

        ImGui::PopID();
      }
      ImGui::EndTable();

      if (d.selectedPage <= 0) {
        ImGui::BeginDisabled();
        ImGui::ArrowButton("Previous", ImGuiDir_Left);
        ImGui::EndDisabled();
      } else if (ImGui::ArrowButton("Previous", ImGuiDir_Left)) {
        d.selectedPage -= 1;
        d.updateSaveData(game);
      }

      int maxPage = (int)levels.size() / 10;
      ImGui::SameLine();
      ImGui::Text("%d / %d", d.selectedPage + 1, maxPage + 1);
      ImGui::SameLine();

      if (d.selectedPage >= maxPage) {
        ImGui::BeginDisabled();
        ImGui::ArrowButton("Next", ImGuiDir_Right);
        ImGui::EndDisabled();
      } else if (ImGui::ArrowButton("Next", ImGuiDir_Right)) {
        d.selectedPage += 1;
        d.updateSaveData(game);
      }
    }
    ImGui::EndPopup();
  }
  if (*p_open) ImGui::OpenPopup(d.popupId.c_str());
  return selected;
}
