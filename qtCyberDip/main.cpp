#include "qtcyberdip.h"
#include <QtWidgets/QApplication>
#include <gameState.h>

#include <random>
#include <chrono>
/*
using namespace cv;

struct ss {
	int i; float a;
};

Mat src; Mat src_gray;
Mat rossiaBlurred; Mat rossia;
Mat dst;
int thresh = 100;
int max_thresh = 255;
//RNG rng(12345);

int threshold_value = 0;
int threshold_type = 3;;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;

void thresh_callback();
void Threshold_Demo(int, void*);
void getChecks(Mat ros);
bool mcomp(const ss &a, const ss &b);
void mswap(int &a, int &b);

int main(int argc, char *argv[])
{
	string dir = "../images";
	string filename = "201751610551.jpg";

	src = cv::imread(dir + "/" + filename);

	std::cout << src.size().width << "   " << src.size().height << endl;

	cvtColor(src, src_gray, CV_BGR2GRAY);
	//GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
	blur(src_gray, src_gray, Size(3, 3));

	thresh_callback();
	qDebug() << 1;
	namedWindow("Source", CV_WINDOW_AUTOSIZE);
	qDebug() << 1;
	imshow("Source", src);

	//createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);


	qDebug() << 1;
	waitKey(0);
	return 0;
}

void thresh_callback()
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// 用Canny算子检测边缘
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);

	//namedWindow("Canny", CV_WINDOW_AUTOSIZE);
	//imshow("Canny", canny_output);
	/// 寻找轮廓
	findContours(canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

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
	Mat contoursImage(src.rows, src.cols, CV_8U, Scalar(255));
	for (int i = 0; i<len; i++) {
		drawContours(contoursImage, contours, sss[i].i, Scalar(0), 3);
	}
	namedWindow("CONTOUR", CV_WINDOW_AUTOSIZE);
	imshow("CONTOUR", contoursImage);

	///  计算中心矩:
	vector<Point2f> mc(len);
	for (int i = 0; i < len; i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	///// 绘出检测结果
	////Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	//int k;
	//for (int i = 0; i< contours.size(); i++)
	//{
	//	//Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	k = sss[i].i;
	//	//drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());

	//	//cout << i << "\tArea: " << area[k] << "\tLength: " << arcLength(contours[i], true) << endl;
	//	cout << i << "\tArea: " << contourArea(contours[k]) << endl;
	//	cout << "mc: " << mc[k] << endl;
	//}

	int background[6];
	int next_background[6];
	int top_button[2] = { 0 };
	int buttons[4];
	enum bg {
		UP, DOWN, LEFT, RIGHT, WIDTH, HEIGHT
	};
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

	for (int n = 0; n < 4; ++n) {
		buttons[n] = 0;
	}

	int tmp_x, tmp_y, index;
	for (int num = 1; num < contours.size(); ++num) {
		index = sss[num].i;
		tmp_x = int(round(mc[index].x));
		tmp_y = int(round(mc[index].y));
		if (tmp_y < background[UP] && tmp_x <= background[LEFT]) {
			if (top_button[0] == 0) top_button[0] = tmp_x;
			if (top_button[1] == 0) top_button[1] = tmp_y;
		}
		if (tmp_y >= background[UP] && tmp_y <= background[DOWN]) {
			if (tmp_x <= background[LEFT] || tmp_x >= background[RIGHT]) {
				if (buttons[LEFT] == 0) {
					buttons[LEFT] = tmp_x;
				}
				else {
					if (abs(tmp_x - buttons[LEFT]) > background[WIDTH]) {
						buttons[RIGHT] = tmp_x;
					}
				}
				if (buttons[UP] == 0) {
					buttons[UP] = tmp_y;
				}
				else {
					if (abs(tmp_y - buttons[UP]) > 1) {
						buttons[DOWN] = tmp_y;
					}
				}
			}
		}
		if (buttons[LEFT] && buttons[RIGHT] && buttons[UP] && buttons[DOWN] && top_button[0] && top_button[1]) break;
	}

	mswap(buttons[LEFT], buttons[RIGHT]);
	mswap(buttons[UP], buttons[DOWN]);

	qDebug() << buttons[LEFT] << ", " << buttons[UP];
	qDebug() << buttons[RIGHT] << ", " << buttons[DOWN];
	qDebug() << top_button[0] << ", " << top_button[1];

	circle(src, Point(buttons[LEFT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	circle(src, Point(buttons[LEFT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(src, Point(buttons[RIGHT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(src, Point(buttons[RIGHT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	circle(src, Point(top_button[0], top_button[1]), 5, Scalar(0), 2, 8, 0);
	rectangle(src, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);

	double block_x = double(background[WIDTH]) / 10;
	double block_y = double(background[HEIGHT]) / 20;
	next_background[LEFT] = background[RIGHT] - round(block_x);
	next_background[RIGHT] = background[RIGHT] + round(3 * block_x);
	next_background[UP] = top_button[1] - round(1.5*block_y);
	next_background[DOWN] = top_button[1] + round(1.5*block_y);
	next_background[WIDTH] = next_background[RIGHT] - next_background[LEFT];
	next_background[HEIGHT] = next_background[DOWN] - next_background[UP];
	rectangle(src, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);

	

	rossiaBlurred = src_gray(Range(background[UP], background[DOWN]), Range(background[LEFT], background[RIGHT]));
	
	getChecks(rossiaBlurred);


	namedWindow("gameState", CV_WINDOW_AUTOSIZE);
	imshow("gameState", rossiaBlurred);
	namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	imshow("Detection", src);
	qDebug() << 1;
	waitKey(0);
	return;
}

void Threshold_Demo(int, void*)
{
	// 0: 二进制阈值
	//1: 反二进制阈值
	//2: 截断阈值
	//3: 0阈值
	//4: 反0阈值
	

	threshold(rossiaBlurred, dst, threshold_value, max_BINARY_value, threshold_type);

	imshow("ROI_GRAY", dst);
}

bool mcomp(const ss &a, const ss &b)
{
	return a.a>b.a;
}

void mswap(int &a, int &b) {
	if (a > b) {
		int tmp = a;
		a = b;
		b = tmp;
	}
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
		//cout << "Best Location: " << dl.loc << '\t' << "Best Rotation:" << dl.rotation << "\t\t" << "Total Deleted Row: " << del_row << endl;
		for (int i = 0; i < dl.rotation; i++) b.rotate();
		state.drop(b, dl.loc);
		//state.show();
		//nb.show();
		type = ntype;
		//cout << "Next Step?";
		//cin >> next;
	}
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	qDebug() << "Total Steps: " << step << endl;
	qDebug() << "Total Deleted Row: " << del_row << endl;
	qDebug() << "Total Times: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms" << endl;
	


	QApplication a(argc, argv);
	qtCyberDip w;
	w.show();
	
	return a.exec();
}
