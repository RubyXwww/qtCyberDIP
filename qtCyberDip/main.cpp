#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>
#include <random>
#include <chrono>

using namespace cv;
/*
struct ss {
	int i; float a;
};
enum BackgroundInformation {
	UP, DOWN, LEFT, RIGHT, WIDTH, HEIGHT
};
bool mcomp(const ss &a, const ss &b)
{
	return a.a>b.a;
}
vector<int> background(6,0);
Mat img; Mat src_gray; Mat src;
void initial();

int main(int argc, char *argv[])
{
	img = imread("../images/20175161107.jpg");
	int thresh = 150;
	Mat canny_output, channel[3];
	vector<Mat> contours(200);
	split(img, channel);
	Canny(channel[1], canny_output, thresh, thresh * 2, 3);
	qDebug() << 1;
	/// Ñ°ÕÒÂÖÀª
	findContours(canny_output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	imshow("canny", canny_output);
	waitKey(0);
	return 0;
}

void initial()
{
	int thresh = 100;
	Mat canny_output, img_gray;
	vector<Mat> contours(300);
	vector<int> stop_button(2, 0);

	cvtColor(img, img_gray, CV_RGB2GRAY);
	/// ÓÃCannyËã×Ó¼ì²â±ßÔµ
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

	/// Ñ°ÕÒÂÖÀª
	findContours(canny_output, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	int len = contours.size();
	vector<ss> sss(len);
	vector<Moments> mu(len);
	for (int i = 0; i < len; i++)
	{
		mu[i] = moments(contours[i], false);
		ss tmp;
		tmp.a = contourArea(contours[i]);
		tmp.i = i;
		sss.push_back(tmp);
	}

	sort(sss.begin(), sss.end(), mcomp);

	///  ¼ÆËãÖÐÐÄ¾Ø:
	vector<Point2f> mc(len);
	for (int i = 0; i < len; i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	vector<Point> frame = contours[sss[0].i];
	int num_points = frame.size();
	background[UP] = frame[0].y;
	background[DOWN] = frame[0].y;
	background[LEFT] = frame[0].x;
	background[RIGHT] = frame[0].x;
	//cout << num_points << endl;
	for (int n = 1; n < num_points; ++n) {
		background[LEFT] = min(background[LEFT], frame[n].x);
		background[RIGHT] = max(background[RIGHT], frame[n].x);
		background[UP] = min(background[UP], frame[n].y);
		background[DOWN] = max(background[DOWN], frame[n].y);
	}

	background[LEFT] += 3; background[UP] += 3;
	background[RIGHT] -= 3; background[DOWN] -= 3;
	background[WIDTH] = background[RIGHT] - background[LEFT];
	background[HEIGHT] = background[DOWN] - background[UP];


	rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	imshow("Detection", img);
}



void getChecks(Mat ros) {
	int width = ros.size().width;
	int height = ros.size().height;

	qDebug() << width << "  " << height;

	int check_x = width / 10;
	int check_y = height / 20;

	int checks[20][10];
	Point coor[20][10];
	coor[0][0].x = check_x / 2 + 5;
	coor[0][0].y = check_y / 2 + 5;

	for (int j = 1; j < 20; ++j) {
		coor[j][0].x = coor[j - 1][0].x;
		coor[j][0].y = coor[j - 1][0].y + check_y;
	}

	for (int j = 0; j < 20; ++j) {
		for (int i = 1; i < 10; ++i) {
			coor[j][i].x = coor[j][i - 1].x + check_x;
			coor[j][i].y = coor[j][i - 1].y;
		}
	}

	for (int j = 0; j < 20; ++j) {
		for (int i = 0; i < 10; ++i) {
			checks[j][i] = (int)ros.at<uchar>(coor[j][i].y, coor[j][i].x);
			circle(ros, Point(coor[j][i].x, coor[j][i].y), 5, Scalar(0), 2, 8, 0);
			cout << checks[j][i] << "\t";
			//cout << coor[j][i].x << "," << coor[j][i].y << "\t";
		}
		cout << endl;
	}
}



*/

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
		//b.show();
		step++;
		//qDebug() << "Best Location: " << dl.loc << '\t' << "Best Rotation:" << dl.rotation << "\t\t" << "Total Deleted Row: " << del_row ;
		//qDebug() << "Drop?";
		//cin >> next;
		for (int i = 0; i < dl.rotation; i++) b.rotate();
		del_row = state.drop(b, dl.loc);
		total_del_row += del_row;
		if (del_row) qDebug() << "Total Deleted Row:" << total_del_row;
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
