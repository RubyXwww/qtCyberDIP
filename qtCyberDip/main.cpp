#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>
#include <random>
#include <chrono>

using namespace cv;
enum BackgroundInformation {
	UP, DOWN, LEFT, RIGHT, WIDTH, HEIGHT
};
vector<int> background(6,0);
vector<int> next_background(6,0);
vector<int> buttons(4,0);
gameState currentState;
struct css {
	int i; float a;
};

bool ccomp(const css &a, const css &b)
{
	return a.a>b.a;
}

void initialLocation(cv::Mat& img) {
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
	vector<css> sss(len);
	vector<Moments> mu(len);
	for (int i = 0; i < len; i++)
	{
		mu[i] = moments(contours[i], false);
		css tmp;
		tmp.a = contourArea(contours[i]);
		tmp.i = i;
		sss.push_back(tmp);
	}

	sort(sss.begin(), sss.end(), ccomp);

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
}

vector<vector<int>> readFromImg(cv::Mat& img, bool isFirst) {
	int erosion_size;
	vector<vector<int>> result(20, vector<int>(10, 0));
	if (isFirst) {
		//qDebug() << "Find first block";
		Mat dst, resize_dst;
		threshold(img, dst, 100, 255, 0);
		resize(dst, resize_dst, Size(10, 20));
		threshold(resize_dst, resize_dst, 200, 255, 0);

		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 10; j++) {
				if ((int)resize_dst.at<uchar>(i, j) == 255) result[i][j] = 1;
			}
		}
	}
	else {
		int erosion_size = 9;
		Mat element = getStructuringElement(MORPH_RECT, Size(2 * erosion_size + 1, 2 * erosion_size + 1), Point(erosion_size, erosion_size));
		Mat erosion_img, resize_dst, dst;
		erode(img, erosion_img, element);
		dst = img - erosion_img;
		threshold(dst, dst, 100, 255, 0);
		resize(dst, resize_dst, Size(10, 20), (0, 0), (0, 0), CV_INTER_AREA);
		threshold(resize_dst, resize_dst, 150, 255, 0);
		resize(resize_dst, img, Size(img.cols, img.rows), 0, 0, INTER_NEAREST);
		//imshow("resize", resize_dst);
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 10; j++) {
				if ((int)resize_dst.at<uchar>(i, j) == 255) result[i][j] = 1;
			}
		}
	}
	return result;

}


void combineAndshow(cv::Mat& pt, cv::Mat& main_area) {
	Mat dst,img;
	pt.copyTo(img);
	int col = main_area.cols;
	int row = main_area.rows;
	for (int i = 1; i < 10; i++) {
		line(main_area, Point(i*col / 10, 0), Point(i*col / 10, row - 1), Scalar(255), 1, 8, 0);
	}
	for (int i = 1; i < 20; i++) {
		line(main_area, Point(0, i*row / 20), Point(col - 1, i*row / 20), Scalar(255), 1, 8, 0);
	}
	cvtColor(main_area, dst, CV_GRAY2RGB);
	dst.copyTo(img(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])));
	namedWindow("Detected", CV_WINDOW_AUTOSIZE);
	imshow("Detected", img);
}

void showState(cv::Mat& pt) {
	Mat state_img(Size(10, 20), CV_8UC1, Scalar(0)), img, dst;
	pt.copyTo(img);
	int col = background[WIDTH];
	int row = background[HEIGHT];
	//imshow("a", state_img);
	for (int i = 0; i < 20; i++) for (int j = 0; j < 10; j++) {
		if (currentState.getState(i, j) == 1) {
			state_img.at<uchar>(i, j) = 255;
		}
	}
	resize(state_img, state_img, Size(col, row), 0, 0, INTER_NEAREST);
	for (int i = 1; i < 10; i++) {
		line(state_img, Point(i*col / 10, 0), Point(i*col / 10, row - 1), Scalar(255), 1, 8, 0);
	}
	for (int i = 1; i < 20; i++) {
		line(state_img, Point(0, i*row / 20), Point(col - 1, i*row / 20), Scalar(255), 1, 8, 0);
	}
	cvtColor(state_img, dst, CV_GRAY2RGB);
	dst.copyTo(img(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])));
	namedWindow("State", CV_WINDOW_AUTOSIZE);
	imshow("State", img);
}

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
		//if (del_row) qDebug() << "Total Deleted Row:" << total_del_row;
		type = ntype;
	}
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	qDebug() << "Total Steps: " << step << endl;
	qDebug() << "Total Deleted Row: " << total_del_row << endl;
	qDebug() << "Total Times: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	
	return 0;
	/*
	Mat pt = imread("../images/blocklabel/0.jpg");
	Mat main_area;
	vector<vector<int>> grid;
	namedWindow("Origin", CV_WINDOW_AUTOSIZE);
	//imshow("Origin", pt);
	initialLocation(pt);
	stringstream ss;
	for (int i = 0; i < 7; i++) {
		ss.str("");
		ss << i;
		pt = imread("../images/blocklabel/"+ss.str()+".jpg");
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_BGR2HSV);
		vector<Mat> mv,mvv;
		split(main_area, mvv);
		Mat test_v = mvv[2];
		main_area = pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]));
		int col, row;
		col = main_area.cols;
		row = main_area.rows;
		split(main_area, mv);
		Mat test(Size(col, row), CV_8UC1, Scalar(0));
		for (int i = 0; i < row; ++i){
			for (int j = 0; j < col; ++j){
				test.at<uchar>(i, j) = max((int)mv[0].at<uchar>(i, j), (int)mv[1].at<uchar>(i, j));
			test.at<uchar>(i, j) = max((int)mv[2].at<uchar>(i, j), (int)test.at<uchar>(i, j));
			}
		}
		imshow("test", test);
		imshow("test_V", test_v);
		grid = readFromImg(test_v, false);
		
		//grid = readFromImg(main_area, false);
		currentState.refresh(grid);
		combineAndshow(pt, test_v);
		//showState(pt);
		imshow("Origin", pt);
		waitKey(0);
	}

	return 0;*/
	/*
	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();

	return a.exec();*/
}
