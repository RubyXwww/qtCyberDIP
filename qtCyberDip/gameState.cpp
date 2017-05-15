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

void gameState::refresh(vector<vector<int>> new_grid) {
	int size = new_grid.size();
	if (size == 0) return;
	if (new_grid[0].size() != 10) return;
	for (int i = 19; i >= max(0, 20 - size); i--) {
		for (int j = 0; j < 10; j++) {
			grid[i][j] = new_grid[size - 20 + i][j];
		}
	}
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
	for (int i = 0; i < 10; i++) heights[i] -= del_row;
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
	double holes;
	double edges;
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
	//edges = accumulate(count_edges.cbegin(), count_edges.cend(), 0);
	//for (int i = 0; i < 20; i++) {
	//	count_edges[i * 10]++;
	//	count_edges[i * 10 + 9]++;
	//}
	for (int i = 0; i < 10; i++) {
		//count_edges[190 + i]++;
		if (grid[19][i] == 0) edges++;
	}
	//holes = count(count_edges.cbegin(), count_edges.cend(), 4);
	holes = total_height - blockNum;
	double val = (200 - total_height * pow(1.1,holes));
	double hval = 10 * (20 - highest_height * pow(1.1, holes));
	return sigmoid(val) + sigmoid(hval) + 10 / edges + pow(1.5, del_row_cur) + pow(1.2, del_row_next);
}

bool gameState::isGameOver() {
	return gameover;
}

DropLoc gameState::getBestDropLoc(gameBlock& cur, gameBlock& next) {
	double best_reward = 0,reward;
	int del_row_cur, del_row_next;
	DropLoc best;
	for (int r = 0; r < cur.rotations; r++) {
		for (int x = 0; x < 10; x++) {
			gameState state_cp(*this);
			del_row_cur = state_cp.drop(cur, x);
			if (state_cp.isGameOver() || del_row_cur == -1) continue;
			for (int nr = 0; nr < next.rotations; nr++) {
				for (int nx = 0; nx < 10; nx++) {
					gameState next_state_cp(state_cp);
					del_row_next = next_state_cp.drop(next, nx);
					if (next_state_cp.isGameOver() || del_row_next == -1) continue;
					reward = next_state_cp.getReward(del_row_cur, del_row_next);
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

/*for test*/
void gameState::show() {
	cout << "Grid:" << endl;
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 10; j++) {
			cout << grid[i][j];
			if (j != 9) cout << ' ';
			else cout << endl;
		}
	}
	cout << "Heights" << endl;
	for (int i = 0; i < 10; i++) {
		cout << heights[i];
		if (i != 9) cout << ' ';
	}
	cout << endl;
	cout << "Reward:\t";
	cout << getReward() << endl;
}
/*end for test*/