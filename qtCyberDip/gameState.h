#pragma once
#include "gameBlock.h"
#include <vector>

struct DropLoc {
	int loc;
	int rotation;
	int del_row;
	DropLoc(int x = 0, int y = 0, int z = 0):loc(x),rotation(y),del_row(z) {}
};

class gameState {
private:
	vector<vector<int>> grid;
	vector<int> heights;
	bool gameover;
public:
	gameState();
	gameState(gameState& tmp);
	void refresh(vector<vector<int>> new_grid);
	int drop(gameBlock& b, int x);
	double getReward(int del_row_cur = 0, int del_row_next = 0);
	bool isGameOver();
	DropLoc getBestDropLoc(gameBlock& first, gameBlock& next);
	/*for test*/
	void show();
	/*end for test*/

};