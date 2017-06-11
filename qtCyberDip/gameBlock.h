#pragma once
#include <vector>

using namespace std;
enum BlockType {
	SLIDE,
	SQUARE,
	FORWARDL,
	REVERSEL,
	FORWARDZ,
	REVERSEZ,
	FLATT,
	NONE
};

struct Pair {
	int height;
	int width;
	Pair(int x = 0, int y = 0):height(x),width(y) {}
};

class gameBlock {
private:
	BlockType type;
	int rotations;
	int current;
	vector<vector<vector<int>>> shapes;
	vector<vector<int>> bottom_locs;
	vector<Pair> hw_pairs;
public:
	friend class gameState;
	gameBlock(BlockType t = BlockType::SQUARE, int c = 0);
	gameBlock(gameBlock& tmp);
	void rotate();
	/*for test*/
	void show();
	void showAll();
	/*end for test*/
};