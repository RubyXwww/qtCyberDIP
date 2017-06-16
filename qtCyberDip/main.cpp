#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>
#include <random>
#include <chrono>

int main(int argc, char *argv[])
{
	/*
	random_device r;
	default_random_engine el(r());
	uniform_int_distribution<int> uniform_dist(0, 6);

	vector<gameBlock> block_vector;
	for (int i = 0; i < 7; i++) {
		BlockType t = (BlockType)i;
		block_vector.push_back(gameBlock(t,0));
	}

	//for (int i = 0; i < 7; i++) block_vector[i].showAll();

	gameState state;
	gameBlock b,nb;
	//state.show();
	int type, ntype;
	DropLoc dl;
	type = uniform_dist(el);
	int del_row = 0,total_del_row = 0, step = 0;
	char next;
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	while (!state.isGameOver()) {
		ntype = uniform_dist(el);
		b = block_vector[type % 7];
		nb = block_vector[ntype % 7];
		dl = state.getBestDropLoc(b);
		//del_row += dl.del_row;
		//state.show();
		//b.showAll();
		step++;
		//qDebug() << "Best Location: " << dl.loc << '\t' << "Best Rotation:" << dl.rotation << "\t\t" << "Total Deleted Row: " << del_row ;
		//qDebug() << "Drop?";
		//cin >> next;
		for (int i = 0; i < dl.rotation; i++) b.rotate();
		del_row = state.drop(b, dl.loc);
		total_del_row += del_row;
		//if (del_row) qDebug() << "Total Deleted Row:" << total_del_row;
		type = ntype;
	}
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	qDebug() << "Total Steps: " << step << endl;
	qDebug() << "Total Deleted Row: " << total_del_row << endl;
	qDebug() << "Total Times: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	
	return 0;*/
	
	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();

	return a.exec();
}
