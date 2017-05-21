#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>
#include <random>
#include <chrono>

int main(int argc, char *argv[])
{
	/*
	for (int i = 0; i < 7; i++) {
		stringstream ss;
		ss << i;
		cv::Mat img = cv::imread(("../images/blocklabel/"+ss.str()+".jpg").c_str());
		cv::Mat roi_img = img(cv::Rect(370, 30, 95, 75));
		cv::imwrite(("../images/blocklabel/" + ss.str() + "_.jpg").c_str(), roi_img);
	}*/
	
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
	type = uniform_dist(el);
	int del_row = 0, step = 0;
	char next;
	chrono::steady_clock::time_point start = chrono::steady_clock::now();
	while (!state.isGameOver()) {
		ntype = uniform_dist(el);
		b = block_vector[type % 7];
		nb = block_vector[ntype % 7];
		dl = state.getBestDropLoc(b,nb);
		del_row += dl.del_row;
		step++;
		cout << "Best Location: " << dl.loc << '\t' << "Best Rotation:" << dl.rotation << "\t\t" << "Total Deleted Row: " << del_row << endl;
		for (int i = 0; i < dl.rotation; i++) b.rotate();
		state.drop(b, dl.loc);
		//state.show();
		//nb.show();
		type = ntype;
		//cout << "Next Step?";
		//cin >> next;
	}
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	cout << "Total Steps: " << step << endl;
	cout << "Total Deleted Row: " << del_row << endl;
	cout << "Total Times: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	
	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();
	
	return a.exec();
}
