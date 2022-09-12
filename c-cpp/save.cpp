#include "game.h"

std::vector<std::string> split(std::string str, char delim) {
  std::vector<std::string> result;
  size_t previous = 0;
  size_t current = str.find(delim);
  while (current != std::string::npos) {
    if (current > previous) {
      result.push_back(str.substr(previous, current - previous));
    }
    previous = current + 1;
    current = str.find(delim, previous);
  }
  if (previous != str.size()) {
    result.push_back(str.substr(previous));
  }
  return result;
}

float Save::getBestScore(const char *game, const char *id) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  try {
    auto d = split(id, '/');
    return save[game][d[0]][d[1]].as<float>();
  } catch (std::runtime_error) {
    return 0.0f;
  }
}

void Save::setBestScore(const char *game, const char *id, float time) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  auto d = split(id, '/');
  save[game][d[0]][d[1]] = time;
  std::ofstream("save.yaml") << save << '\n';
}

bool Save::getLevelSelectData(const char *game, std::string *tid, int *page) {
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

void Save::setLevelSelectData(const char *game, std::string tid, int page) {
  YAML::Node save = YAML::LoadFile("save.yaml");
  save["levelSelect"][game]["tid"] = tid;
  save["levelSelect"][game]["page"] = page;
  std::ofstream("save.yaml") << save << '\n';
}
