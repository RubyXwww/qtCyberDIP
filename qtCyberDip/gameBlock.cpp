#include <gameBlock.h>
#include <iostream>

gameBlock::gameBlock(BlockType t, int c) {
	type = t;
	current = c;
	vector<vector<int>> shape(4, vector<int>(4, 0));
	switch (type) {
	case SLIDE:
		rotations = 2;
		shape[3] = vector<int>(4, 1);
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(1, 4));
		bottom_locs.push_back(vector<int>(4, 0));
		shape[3] = vector<int>(4, 0);
		for (int i = 0; i < 4; i++) shape[i][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(4, 1));
		bottom_locs.push_back(vector<int>(1, 0));
		break;

	case SQUARE:
		rotations = 1;
		shape[3][0] = 1;
		shape[3][1] = 1;
		shape[2][0] = 1;
		shape[2][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 2));
		bottom_locs.push_back(vector<int>(2, 0));
		break;

	case FORWARDL:
		rotations = 4;
		shape[1][0] = 1;
		shape[1][1] = 1;
		shape[2][1] = 1;
		shape[3][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 2,0 }));
		shape[1][0] = 0;
		shape[1][1] = 0;
		shape[2][1] = 0;
		shape[3][0] = 1;
		shape[3][2] = 1;
		shape[2][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>(3,0));
		shape[3][2] = 0;
		shape[2][2] = 0;
		shape[2][0] = 1;
		shape[1][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>(2,0));
		shape[1][0] = 0;
		shape[3][1] = 0;
		shape[2][1] = 1;
		shape[2][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 0,1,1 }));
		break;

	case REVERSEL:
		rotations = 4;
		shape[1][0] = 1;
		shape[1][1] = 1;
		shape[2][0] = 1;
		shape[3][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 0,2 }));
		shape[1][0] = 0;
		shape[1][1] = 0;
		shape[3][0] = 0;
		shape[2][1] = 1;
		shape[2][2] = 1;
		shape[3][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 1,1,0 }));
		shape[3][2] = 0;
		shape[2][2] = 0;
		shape[2][0] = 0;
		shape[3][0] = 1;
		shape[3][1] = 1;
		shape[1][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>(2, 0));
		shape[1][1] = 0;
		shape[2][1] = 0;
		shape[2][0] = 1;
		shape[3][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>(3, 0));
		break;

	case FORWARDZ:
		rotations = 2;
		shape[2][0] = 1;
		shape[2][1] = 1;
		shape[3][1] = 1;
		shape[3][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 1,0,0 }));
		shape[3][1] = 0;
		shape[3][2] = 0;
		shape[3][0] = 1;
		shape[1][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 0,1 }));
		break;

	case REVERSEZ:
		rotations = 2;
		shape[2][2] = 1;
		shape[2][1] = 1;
		shape[3][1] = 1;
		shape[3][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 0,0,1 }));
		shape[3][0] = 0;
		shape[2][2] = 0;
		shape[2][0] = 1;
		shape[1][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 1,0 }));
		break;

	case FLATT:
		rotations = 4;
		shape[2][0] = 1;
		shape[2][1] = 1;
		shape[2][2] = 1;
		shape[3][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 1,0,1 }));
		shape[2][2] = 0;
		shape[1][1] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 1,0 }));
		shape[1][1] = 0;
		shape[2][0] = 0;
		shape[3][0] = 1;
		shape[3][2] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(2, 3));
		bottom_locs.push_back(vector<int>({ 0,0,0 }));
		shape[3][1] = 0;
		shape[3][2] = 0;
		shape[2][0] = 1;
		shape[1][0] = 1;
		shapes.push_back(shape);
		hw_pairs.push_back(Pair(3, 2));
		bottom_locs.push_back(vector<int>({ 0,1 }));
		break;

	default:
		break;
	}
}

gameBlock::gameBlock(gameBlock& tmp) {
	type = tmp.type;
	rotations = tmp.rotations;
	current = tmp.current;
	shapes = tmp.shapes;
	bottom_locs = tmp.bottom_locs;
	hw_pairs = tmp.hw_pairs;
}

void gameBlock::rotate() {
	current = (current + 1) % rotations;
}
/*for test*/
void gameBlock::show() {
	qDebug() << "Block rotation:" << current;
	for (int j = 0; j < 4; j++) {
		qDebug() << shapes[current][j][0] << shapes[current][j][1] << shapes[current][j][2] << shapes[current][j][3];
	}
}

void gameBlock::showAll() {
	for (int i = 0; i < rotations; i++) {
		qDebug() << "Block rotation:" << i;
		vector<vector<int>> tmp = shapes[i];
		for (int j = 0; j < 4; j++) {
			qDebug() << shapes[i][j][0] << shapes[i][j][1] << shapes[i][j][2] << shapes[i][j][3];
		}
		qDebug() << "bottom loc:";
		for (int h : bottom_locs[i]) qDebug() << h;
		qDebug() << "Height: " << hw_pairs[i].height;
		qDebug() << "Width: " << hw_pairs[i].width;

		//cout << '\n';
	}
}
/*end for test*/