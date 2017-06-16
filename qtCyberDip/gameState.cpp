#include "gameState.h"
#include <iostream>
#include <algorithm>
#include <cmath>

double sigmoid(double x) {
	return 1 / (1 + exp(0 - x));
}

gameState::gameState() {
	grid = vector<vector<int>>(20, vector<int>(10, 0));
	heights = vector<int>(10, 0);
	gameover = false;
}

gameState::gameState(gameState& tmp) {
	grid = tmp.grid;
	heights = tmp.heights;
	gameover = tmp.gameover;
}

void gameState::clear() {
	grid = vector<vector<int>>(20, vector<int>(10, 0));
	heights = vector<int>(10, 0);
	gameover = false;
}

void gameState::refreshHeights() {
	for (int j = 0; j < 10; j++) {
		heights[j] = 20;
		while (heights[j] > 0 && grid[20 - heights[j]][j] == 0) heights[j]--;
	}
}

void gameState::refresh(vector<vector<int>> new_grid) {
	int size = new_grid.size();
	if (size == 0) return;
	if (new_grid[0].size() != 10) return;
	int count;
	bool flag = false;
	for (int i = 19; i > max(0, 20 - size); i--) {
		if (!flag) {
			count = 0;
			for (int j = 0; j < 10; j++) {
				count += new_grid[size - 20 + i][j];
				grid[i][j] = new_grid[size - 20 + i][j];
			}
			if (count == 0) flag = true;
		}
		else {
			for (int j = 0; j < 10; j++) {
				grid[i][j] = 0;
			}
		}
	}
	refreshHeights();
}

int gameState::drop(gameBlock& b, int x) {
	vector<vector<int>> shape = b.shapes[b.current];
	vector<int> loc = b.bottom_locs[b.current];
	int height = b.hw_pairs[b.current].height;
	int width = b.hw_pairs[b.current].width;
	if (x + width > 10) return -1;
	int y = 19;
	for (int i = 0; i < loc.size(); i++) {
		y = min(y, 19 - heights[x + i] + loc[i]);
	}
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if( y - i >= 0 ) grid[y - i][x + j] += shape[3 - i][j];
		}
	}
	for (int i = 0; i < width; i++) {
		int index = 20;
		while (index > 0 && grid[20 - index][x + i] == 0) index--;
		heights[x + i] = index;
	}
	if (y - height < -1) {
		gameover = true;
		return 0;
	}
	int del_row = 0;
	for (int i = y; i > max(-1, y - height); i--) {
		if (accumulate(grid[i].begin(), grid[i].end(), 0) == 0) break;
		if (accumulate(grid[i].begin(), grid[i].end(), 0) == 10) {
			for (int j = i; j > 0; j--) {
				grid[j] = grid[j - 1];
			}
			grid[0] = vector<int>(10, 0);
			i++;
			del_row++;
		}
	}
	for (int i = 0; i < 10; i++) {
		heights[i] -= del_row;
		while (heights[i] > 0 && grid[20 - heights[i]][i] == 0) heights[i]--;
	}
	return del_row;
}

int gameState::drop_PD(gameBlock& b, int x, double& avg_land_height) {
	vector<vector<int>> shape = b.shapes[b.current];
	vector<int> loc = b.bottom_locs[b.current];
	int height = b.hw_pairs[b.current].height;
	int width = b.hw_pairs[b.current].width;
	if (x + width > 10) return -1;
	int y = 19;
	for (int i = 0; i < loc.size(); i++) {
		y = min(y, 19 - heights[x + i] + loc[i]);
	}
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (y - i >= 0) grid[y - i][x + j] += shape[3 - i][j];
		}
	}
	for (int i = 0; i < width; i++) {
		int index = 20;
		while (index > 0 && grid[20 - index][x + i] == 0) index--;
		heights[x + i] = index;
	}
	if (y - height < -1) {
		gameover = true;
		return 0;
	}
	int del_row = 0;
	//int provide = 0;
	for (int i = y; i > max(-1, y - height); i--) {
		if (accumulate(grid[i].begin(), grid[i].end(), 0) == 0) break;
		if (accumulate(grid[i].begin(), grid[i].end(), 0) == 10) {
			for (int j = i; j > 0; j--) {
				grid[j] = grid[j - 1];
			}
			//provide += accumulate(shape[3 - y + i].begin(), shape[3 - y + i].end(), 0);
			grid[0] = vector<int>(10, 0);
			i++;
			del_row++;
		}
	}
	avg_land_height = 19 - y + double(height - 1) / 2;
	for (int i = 0; i < 10; i++) {
		heights[i] -= del_row;
		while (heights[i] > 0 && grid[20 - heights[i]][i] == 0) heights[i]--;
	}
	return del_row;
}

double gameState::getReward(int del_row_cur, int del_row_next) {
	int d_x[4] = { 1,-1,0,0 };
	int d_y[4] = { 0,0,1,-1 };
	//vector<int> count_edges(200, 0);
	int n_x, n_y;
	double highest_height = *max_element(heights.cbegin(), heights.cend());
	double total_height = accumulate(heights.cbegin(),heights.cend(),0);
	double avg_height = total_height / heights.size();
	double diff = 0, dep = 0, alp = 0;
	int edges = 0;
	bool flag;
	int blockNum = 0;
	for (int i = 19; i >= 0; i--) {
		flag = true;
		for (int j = 0; j < 10; j++) {
			if (grid[i][j] == 1) {
				blockNum++;
				flag = false;
				for (int k = 0; k < 4; k++) {
					n_x = i + d_x[k];
					n_y = j + d_y[k];
					if (n_x < 0 || n_x > 19) continue;
					if (n_y < 0 || n_y > 9) continue;
					if (grid[n_x][n_y] == 0) edges++;
				}
			}
		}
		if (flag) break;
	}


	for (int i = 0; i < 10; i++) {
		if (grid[19][i] == 0) edges++;
		if (i != 9) diff += abs(heights[i] - heights[i + 1]);

		if (i != 0 && i != 9) {
			if (heights[i] - heights[i - 1] > 1 && heights[i] - heights[i + 1] > 1) alp += 1;
			if (heights[i] - heights[i - 1] < -1 && heights[i] - heights[i + 1] < -1) dep += 1;
		}
	}
	/*
	total_height: 高度总和
	blockNum: 方块综合
	holes: 洞的数量（通过下落无法填补的方块）
	highest_height: 最高的高度
	avg: 平均高度
	del_row_cur: 第一个俄罗斯方块消除行数
	del_row_next: 第二个俄罗斯方块消除行数
	dep: 深坑数量（与左右高度差都小于等于-2）
	alp: 山峰数量（与左右高度差都大于等于2）
	diff: 高度差总和
	edges: 边缘总数
	*/
	double Row_Transitions = getRowTransitions(); //行变换数
	double Column_Transitions = getColumnTransitions(); //列变换数
	double Number_of_Holes = getNumberofHoles(); //空洞数
	double Well_Sums = getWellSums(); //井数

	double val = (200 - total_height * pow(1.1, Number_of_Holes)) / 200;
	double hval = (20 - highest_height * pow(1.1, Number_of_Holes)) / 20;
	double avg = total_height / 10;
	double highest = (20 - highest_height);
	return sigmoid(val) + sigmoid(hval) + 10/edges + pow(1.5, del_row_cur) + pow(1.25, del_row_next);
	//return highest * 19 + avg * 15 + del_row_cur * 2 + del_row_next * 1 - holes * 7 - dep * 4 - alp * 2 - diff;
}

double gameState::getRowTransitions() {
	double Row_Transitions = 0;
	for (int i = 19; i > -1/*19 - *max_element(heights.cbegin(), heights.cend())*/; i--) {
		int state = 1;
		for (int j = 0; j < 10; j++) {
			if (grid[i][j] != state) {
				Row_Transitions += 1;
				state = grid[i][j];
			}
		}
		if (state == 0) Row_Transitions += 1;
	}
	return Row_Transitions;
}

double gameState::getColumnTransitions() {
	double Column_Transitions = 0;
	for (int j = 0; j < 10; j++) {
		int state = 1;
		for (int i = 0; i < 20; i++) {
			if (grid[i][j] != state) {
				Column_Transitions += 1;
				state = grid[i][j];
			}
		}
		//if (state == 0) Column_Transitions += 1;
	}
	return Column_Transitions;
}

double gameState::getNumberofHoles() {
	int blockNum = 0;
	for (int j = 0; j < 10; j++) for (int i = 19; i > 19 - heights[j]; i--) blockNum += grid[i][j];
	return accumulate(heights.cbegin(), heights.cend(), 0) - blockNum;
}

double gameState::getWellSums() {
	double Well_Sums = 0;
	for (int j = 0; j < 10; j++) {
		int count = 1;
		if (j == 0) {
			for (int i = 19; i > 19 - heights[j + 1]; i--) {
				if (grid[i][j] == 0 && grid[i][j + 1] == 1) Well_Sums += count++;
				else count = 1;
			}
		}
		else if (j == 9) {
			for (int i = 19; i > 19 - heights[j - 1]; i--) {
				if (grid[i][j] == 0 && grid[i][j - 1] == 1) Well_Sums += count++;
				else count = 1;
			}
		}
		else {
			for (int i = 19; i > 19 - min(heights[j + 1], heights[j - 1]); i--) {
				if (grid[i][j] == 0 && grid[i][j - 1] == 1 && grid[i][j + 1] == 1) Well_Sums += count++;
				else count = 1;
			}
		}
	}
	return Well_Sums;
}

double gameState::getReward_PD(int del_row, double land_height) {
	double Landing_Height = land_height; //下落高度
	double Row_eliminated = del_row; //消行数
	double Row_Transitions = getRowTransitions(); //行变换数
	double Column_Transitions = getColumnTransitions(); //列变换数
	double Number_of_Holes = getNumberofHoles(); //空洞数
	double Well_Sums = getWellSums(); //井数
	vector<double> parameters = { -4.500158825082766,\
								   3.4181268101392694,\
								  -3.2178882868487753,\
								  -9.348695305445199,\
								  -7.899265427351652,\
								  -3.3855972247263626 };
	double score = 0;
	
	score += parameters[0] * Landing_Height;
	score += parameters[1] * Row_eliminated;
	score += parameters[2] * Row_Transitions;
	score += parameters[3] * Column_Transitions;
	score += parameters[4] * Number_of_Holes;
	score += parameters[5] * Well_Sums;
	//qDebug() << score;
	//return score;
	return parameters[0] * Landing_Height +\
		parameters[1] * Row_eliminated +\
		parameters[2] * Row_Transitions +\
		parameters[3] * Column_Transitions +\
		parameters[4] * Number_of_Holes +\
		parameters[5] * Well_Sums;
}

bool gameState::isGameOver() {
	return gameover;
}

int gameState::getState(int x, int y) {
	return grid[x][y];
}

DropLoc gameState::getBestDropLoc(gameBlock& cur, gameBlock& next) {
	double best_reward = -1e32,reward;
	int del_row_cur, del_row_next;
	DropLoc best;
	double land_height;
	for (int r = 0; r < cur.rotations; r++) {
		for (int x = 0; x < 11 - cur.hw_pairs[r].width; x++) {
			gameState state_cp(*this);
			del_row_cur = state_cp.drop(cur, x);
			if (state_cp.isGameOver()) continue;
			for (int nr = 0; nr < next.rotations; nr++) {
				for (int nx = 0; nx < 11 - next.hw_pairs[nr].width; nx++) {
					gameState next_state_cp(state_cp);
					del_row_next = next_state_cp.drop_PD(next, nx, land_height);
					if (next_state_cp.isGameOver()) continue;
					//reward = next_state_cp.getReward(del_row_cur, del_row_next);
					reward = next_state_cp.getReward_PD(del_row_next+del_row_cur, land_height);
					if (reward > best_reward) {
						best_reward = reward;
						best.loc = x;
						best.rotation = r;
						best.del_row = del_row_cur;
					}
				}
				next.rotate();
			}
		}
		cur.rotate();
	}
	return best;
}

DropLoc gameState::getBestDropLoc(gameBlock& cur) {
	double best_reward = -100000, reward;
	int del_row_cur;
	double land_height;
	int priority, best_priority;
	DropLoc best;
	for (int r = 0; r < cur.rotations; r++) {
		for (int x = 0; x < 11 - cur.hw_pairs[r].width; x++) {
			gameState state_cp(*this);
			del_row_cur = state_cp.drop_PD(cur, x, land_height);
			if (state_cp.isGameOver()) continue;
			reward = state_cp.getReward_PD(del_row_cur, land_height);
			if (x < 5) priority = 5 - x;
			else priority = x - 5;
			if (r > 0 && x < 5) priority++;
			priority += r;
			if (reward > best_reward) {
				best_reward = reward;
				best.loc = x;
				best.rotation = r;
				best_priority = priority;
				best.del_row = del_row_cur;
			}
			else if (reward == best_reward && priority < best_priority) {
				best_reward = reward;
				best.loc = x;
				best.rotation = r;
				best_priority = priority;
				best.del_row = del_row_cur;
			}
		}
		cur.rotate();
	}
	//qDebug() << "Best reward: " << best_reward;
	return best;
}
/*for test*/
void gameState::show() {
	qDebug() << "Grid:";
	for (int i = 0; i < 20; i++) {
		qDebug() << grid[i][0] << grid[i][1] << grid[i][2] << grid[i][3] << grid[i][4] << grid[i][5] << grid[i][6] << grid[i][7] << grid[i][8] << grid[i][9];
	}
	qDebug() << "Heights";
	qDebug() << heights[0] << heights[1] << heights[2] << heights[3] << heights[4] << heights[5] << heights[6] << heights[7] << heights[8] << heights[9];
	qDebug() << "Row_Transitions: " << getRowTransitions(); //行变换数
	qDebug() << "Column_Transitions: " << getColumnTransitions(); //列变换数
	qDebug() << "Number_of_Holes" << getNumberofHoles(); //空洞数
	qDebug() << "Well_Sums" << getWellSums(); //井数
}
/*end for test*/