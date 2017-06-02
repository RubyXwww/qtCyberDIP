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
	void clear();
	void refreshHeights();
	void refresh(vector<vector<int>> new_grid);
	double getRowTransitions(); //行变换数
	double getColumnTransitions(); //列变换数
	double getNumberofHoles(); //空洞数
	double getWellSums(); //井数
	int drop(gameBlock& b, int x);
	int drop_PD(gameBlock& b, int x, double& avg_land_height);
	double getReward(int del_row_cur = 0, int del_row_next = 0);
	double getReward_PD(int del_row_cur = 0, double land_height_cur = 19);
	bool isGameOver();
	int getState(int x, int y);
	DropLoc getBestDropLoc(gameBlock& first, gameBlock& next);
	DropLoc getBestDropLoc(gameBlock& first);
	/*for test*/
	void show();
	/*end for test*/

};