#include "internal.h"

void Internal::TimeText(int time) {
  ImGui::Text("%02d:%02d", time / 60, time % 60);
}

void Internal::TimeText(float time) { Internal::TimeText((int)time); }

bool Internal::Difficulty(int difficulty) {
  static char difficulties[4][10]{"Easy", "Medium", "Hard", "V.Hard"};
  ImGui::PushStyleButtonColored(3 - difficulty, 7);
  bool pressed = ImGui::Button(difficulties[difficulty], {60, 0});
  ImGui::PopStyleButtonColored();
  return pressed;
}

void Internal::TutorialWindow(const char *game, bool *p_open) {
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
      Internal::getLevelSelectData(game, &selectedTId, &selectedPage);
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
                        Internal::getBestTime(game, id.c_str())});
    }

    categories.push_back({tid, origin, levels});
  }
}

GameCategory &LevelSelectData::selectedCategory() {
  return categories[selectedCategoryIndex];
}

void LevelSelectData::updateSaveData(const char *game) {
  Internal::setLevelSelectData(game, selectedCategory().tid, selectedPage);
}

bool Internal::LevelSelectWindow(const char *game, bool *p_open,
                                 std::string *p_level) {
  bool selected = false;
  static LevelSelectData d{game};

  static bool init = true;
  if (init) {
    d = LevelSelectData{game};
    init = false;
  }

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
        if (Internal::Difficulty(level.difficulty)) {
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
          Internal::TimeText(level.bestTime);
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
  if (*p_open)
    ImGui::OpenPopup(d.popupId.c_str());
  else
    init = true;
  return selected;
}

float Internal::getBestTime(const char *game, const char *id) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  try {
    auto d = split(id, '/');
    return save[game][d[0]][d[1]].as<float>();
  } catch (std::runtime_error) {
    return 0.0f;
  }
}

void Internal::setBestTime(const char *game, const char *id, float time) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  auto d = split(id, '/');
  save[game][d[0]][d[1]] = time;
  std::ofstream("save.yaml") << save << '\n';
}

bool Internal::getLevelSelectData(const char *game, std::string *tid,
                                  int *page) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  try {
    *tid = save["levelSelect"][game]["tid"].as<std::string>();
    *page = save["levelSelect"][game]["page"].as<int>();
    return true;
  } catch (std::runtime_error) {
    *tid = "";
    *page = 0;
    return false;
  }
}

void Internal::setLevelSelectData(const char *game, std::string tid, int page) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  save["levelSelect"][game]["tid"] = tid;
  save["levelSelect"][game]["page"] = page;
  std::ofstream("save.yaml") << save << '\n';
}
