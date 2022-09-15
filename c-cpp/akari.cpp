#include "game.h"

using AkariOperation = NumberOperation;

struct Akari :public Puzzle<AkariOperation> {
	int width, height;
	int errX, errY;
	std::vector<std::vector<int>> Board;
	Akari(const char* id);
	bool isRelated(int x1, int y1, int x2, int y2);
	void input(int x, int y, int num);
	//void check();
};

void AkariWindow(bool* p_open) {
	static Akari akari{ "test/1" };
	if (!ImGui::Begin("Akari", p_open,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_MenuBar))
		return ImGui::End();  // 如果窗口被折叠, 则直接短路不计算窗口内容.

	ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });

	// 窗口顶部的菜单栏.
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Game")) {
			if (ImGui::MenuItem("Reset")) akari = Akari{ akari.id.c_str() };
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("Tutorial")) {
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	static int num = -2;
	static int hoveredX = -1;
	static int hoveredY = -1;

	ImGui::BeginChild(
		"AkariChild",
		{ GetRealWidth(CellSize, akari.width, true), GetRealHeight(CellSize, akari.height, true) }, true);
	const auto pos = ImGui::GetCursorScreenPos();
	const auto col = ImGui::BorderColor();
	const auto bac = ImGui::BackgroundColor();
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	for (int y = 0; y < akari.height; y++)
	{
		for (int x = 0; x < akari.width; x++)
		{
			if (x != 0) ImGui::SameLine();
			ImGui::PushID(x + y * akari.width);
			const auto vec = ImGui::GetPos(pos, x + 1, y + 1);
			for (int i = 0; i <= 3; i++) ImGui::DrawBorder(drawList, pos, x + 1, y + 1, i, bac);
			if (akari.Board[y][x] != -1)
			{
				if (akari.Board[y][x] != -2 && akari.Board[y][x] != -3)
				{
					ImGui::DrawRectFilled(drawList, vec, ImGui::BackgroundColor(0.5f));
					ImGui::Selectable(std::to_string(akari.Board[y][x]).c_str(), akari.isRelated(hoveredX, hoveredY, x, y), 0, ImVec2(CellSize, CellSize));
				}
				else
				{
					if (akari.Board[y][x] == -3)
						ImGui::DrawRectFilled(drawList, vec);
					if (akari.Board[y][x] == -2)
					{
						ImVec2 vec2;
						vec2.x = (vec.x + vec.z) / 2;
						vec2.y = (vec.y + vec.w) / 2;
						ImGui::DrawRectFilled(drawList, vec);
						ImGui::DrawCircleFilled(drawList, vec2, (float)CellSize / 2);
					}
					ImGui::Selectable("", akari.isRelated(hoveredX, hoveredY, x, y), 0, ImVec2(CellSize, CellSize));
				}
				//drawList->AddText({ vec.x,vec.w }, col, std::to_string(akari.Board[y][x]).c_str());
				//ImGui::Selectable(std::to_string(akari.Board[y][x]).c_str(), akari.isRelated(hoveredX, hoveredY, x, y), 0, ImVec2(CellSize, CellSize));
			}
			else
			{
				if (ImGui::Selectable("", akari.isRelated(hoveredX, hoveredY, x, y), 0, ImVec2(CellSize, CellSize)))
				{
					//akari.input(x+1, y+1, -3);
					akari.input(hoveredX, hoveredY, -2);
					for (int x1 = hoveredX + 1; x1 < akari.width; x1++)
					{
						if (akari.Board[hoveredY][x1] == -3) continue;
						if (akari.Board[hoveredY][x1] == -1) akari.input(x1, hoveredY, -3);
						else break;
					}
					for (int x1 = hoveredX - 1; x1 >= 0; x1--)
					{
						if (akari.Board[hoveredY][x1] == -3) continue;
						if (akari.Board[hoveredY][x1] == -1) akari.input(x1, hoveredY, -3);
						else break;
					}
					for (int y1 = hoveredY + 1; y1 < akari.height; y1++)
					{
						if (akari.Board[y1][hoveredX] == -3) continue;
						if (akari.Board[y1][hoveredX] == -1) akari.input(hoveredX, y1, -3);
						else break;
					}
					for (int y1 = hoveredY - 1; y1 >= 0; y1--)
					{
						if (akari.Board[y1][hoveredX] == -3) continue;
						if (akari.Board[y1][hoveredX] == -1) akari.input(hoveredX, y1, -3);
						else break;
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				hoveredX = x;
				hoveredY = y;
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	akari.LevelDetails("<Error>");
	ImGui::PopStyleVar();
	ImGui::End();
}

Akari::Akari(const char* id) : Puzzle{ "akari", id }, errX{ 0 }, errY{ 0 } {
	width = node["width"].as<int>();
	height = node["height"].as<int>();
	std::vector<int> emptyRow{};
	for (int x = 0; x < width; x++) emptyRow.push_back(0);
	for (int y = 0; y < height; y++)
	{
		Board.push_back(emptyRow);
		for (int x = 0; x < width; x++)
		{
			int board = node["board"][y][x].as<int>();
			Board[y][x] = board - 1;
		}
	}
}

void Akari::input(int x, int y, int num) {
	if (Board[y][x] == num) return;
	Board[y][x] = num;
}

bool Akari::isRelated(int x1, int y1, int x2, int y2) {
	if (x1 == -1 || y1 == -1) return false;
	if (Board[y1][x1] != -1 && Board[y1][x1] != -3 && Board[y1][x1] != -2) return false;
	if (x1 == x2) {
		for (int y = MIN(y1, y2); y <= MAX(y1, y2); y++)
		{
			if (Board[y][x1] != -1 && Board[y][x1] != -3 && Board[y][x1] != -2) return false;
		}
	}
	else if (y1 == y2) {
		for (int x = MIN(x1, x2); x <= MAX(x1, x2); x++)
		{
			if (Board[y1][x] != -1 && Board[y1][x] != -3 && Board[y1][x] != -2) return false;
		}
	}
	else
		return false;
	return true;
}