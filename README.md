# 基于 C/C++ 语言的编程设计
## 目前进度
- 项目未确定 (︶︹︺)
- 决定先用 [`Dear ImGui`](https://github.com/ocornut/imgui) 写点益智小游戏试试手.
  - 游戏关卡以 `yaml` 格式保存并利用 [`yaml-cpp`](https://github.com/jbeder/yaml-cpp) 库进行解析.
  - 基本完成了 Sudoku (数独) 游戏, 可作为开发其他游戏的示例. 见 `game.h`, `sudoku.cpp` 与 `/levels/sudoku`.
  - 基本完成了 Kakuro (数和) 游戏.

## 初始化
将项目克隆到本地:

```sh
git clone https://github.com/waterv/c-cpp.git
```

### 自动格式化
安装 `clang-format` 格式化工具:

```sh
# Linux
sudo apt install clang-format

# macOS
brew install clang-format

# Windows 可直接下载 LLVM 二进制文件安装,
# 安装时注意勾选 'Add LLVM to the system PATH'
```

修改 git 钩子的路径:

```sh
git config core.hooksPath .githooks
```
