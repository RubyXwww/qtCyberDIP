#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>
#include <random>
int main(int argc, char *argv[])
{
	random_device r;
	default_random_engine el(r());
	uniform_int_distribution<int> uniform_dist(0, 6);

	vector<gameBlock> block_vector;
	for (int i = 0; i < 7; i++) {
		BlockType t = (BlockType)i;
		block_vector.push_back(gameBlock(t,0));
	}

	gameState state;
	gameBlock b,nb;
	state.show();
	int type, ntype;
	DropLoc dl;
	/*
	cout << "First Block Type: ";
	cin >> type;*/
	type = uniform_dist(el);
	int del_row = 0;
	char next;
	while (!state.isGameOver()) {
		/*
		cout << "Next Block Type: ";
		cin >> ntype;*/
		ntype = uniform_dist(el);
		b = block_vector[type % 7];
		nb = block_vector[ntype % 7];
		dl = state.getBestDropLoc(b,nb);
		del_row += dl.del_row;
		cout << "Best Location: " << dl.loc << '\t' << "Best Rotation:" << dl.rotation << "\t\t" << "Total Deleted Row: " << del_row << endl;
		for (int i = 0; i < dl.rotation; i++) b.rotate();
		state.drop(b, dl.loc);
		//state.show();
		//nb.show();
		type = ntype;
		//cout << "Next Step?";
		//cin >> next;
	}

	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();
	
	return a.exec();
}
