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

void Game::TutorialWindow(const char *name, bool *p_open) {
  ImGui::Begin((std::string("Tutorial###Tutorial of ") + name).c_str(), p_open,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_AlwaysAutoResize);
  auto node = YAML::LoadFile(std::string("../levels/") + name +
                             "/info.yaml")["tutorials"];
  if (ImGui::BeginTabBar((std::string("Tutorial Tabbar of ") + name).c_str(),
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

bool Game::LevelSelectWindow(const char *name, bool *p_open,
                             std::string *p_level) {
  bool selected = false;
  std::string popupId = std::string("Level Select###Level Select of ") + name;

  if (ImGui::BeginCenterPopupModal(popupId.c_str(), p_open)) {
    auto node = YAML::LoadFile(std::string("../levels/") + name +
                               "/info.yaml")["levels"];

    if (ImGui::BeginTable((std::string("Level Table of ") + name).c_str(), 4)) {
      ImGui::TableSetupColumn("Difficulty");
      ImGui::TableSetupColumn("ID");
      ImGui::TableSetupColumn("Author");
      ImGui::TableSetupColumn("From");
      ImGui::TableHeadersRow();

      int idNum = 0;
      for (size_t i = 0; i < node.size(); i++) {
        auto prefix = node[i]["prefix"].as<std::string>();
        auto from = node[i]["origin"].as<std::string>();
        auto n = node[i]["number"].as<int>();
        for (int j = 1; j <= n; j++, idNum++) {
          auto id = prefix + std::to_string(j);
          auto level = YAML::LoadFile(std::string("../levels/") + name + "/" +
                                      id + ".yaml");
          auto author = level["author"].as<std::string>();
          auto difficulty = level["difficulty"].as<int>();

          ImGui::PushID(idNum);
          ImGui::TableNextRow();

          ImGui::TableSetColumnIndex(0);
          if (ImGui::Difficulty(difficulty)) {
            selected = true;
            *p_open = false;
            *p_level = id;
          }

          ImGui::TableSetColumnIndex(1);
          ImGui::Text(id.c_str());

          ImGui::TableSetColumnIndex(2);
          ImGui::Text(author.c_str());

          ImGui::TableSetColumnIndex(3);
          ImGui::Text(from.c_str());

          ImGui::PopID();
        }
      }
      ImGui::EndTable();
    }
    ImGui::EndPopup();
  }

  if (*p_open) ImGui::OpenPopup(popupId.c_str());
  return selected;
}
