#include "usrGameController.h"
#include <time.h>
#include <sstream>
#include<fstream>
#ifdef VIA_OPENCV

#define DELAY_STEP 42
#define FAIL_STEP 12
#define CHECK_STEP 12

//鏋勯�犱笌鍒濆鍖�
using namespace cv;

struct ss {
	int i; float a;
};

bool comp(const ss &a, const ss &b)
{
	return a.a>b.a;
}

bool compp(const int &a, const int &b){
	return a < b;
}

void swap(int &a, int &b) {
	if (a > b) {
		int tmp = a;
		a = b;
		b = tmp;
	}
}

usrGameController::usrGameController(void* qtCD)
{
	qDebug() << "usrGameController online.";
	device = new deviceCyberDip(qtCD);//璁惧浠ｇ悊绫�
	cv::namedWindow(WIN_NAME);
	
	ifstream fin("../images/blocklabel/features.txt");
	double tmp;
	qDebug() << "Loading features...";
	for (int i = 0; i < 7; i++) {
		vector<double> feature;
		for (int j = 0; j < 12; j++) {
			fin >> tmp;
			feature.push_back(tmp);
		}
		blockFeatures.push_back(feature);
	}
	fin.close();
	menuFeature = { 0.352975, 0.312813, 0.334212, 0.339132, 0.315268, 0.3456, 0.355629, 0.312471, 0.3319, 0.330162, 0.316513, 0.353307 };
	pt_cols = 0;
	pt_rows = 0;
	background = vector<int>(6, 0);
	next_background = vector<int>(6, 0);
	//buttons = vector<int>(4, 0);
	buttons = vector<cv::Point>(5);
	start_button = vector<int>(2, 0);
	isFirstBlock = false;
	jump_flag = false;
	Op_delay = 1000;
	currentRecursiveState = NEWGAME;
	lastState = NEWCYCLE;
	isGame = false;
	isInitialed = false;

	blockMap[0x000f] = Tmp(SLIDE, 0);
	blockMap[0x8888] = Tmp(SLIDE, 1);
	blockMap[0x00cc] = Tmp(SQUARE, 0);
	blockMap[0x0c44] = Tmp(FORWARDL, 0);
	blockMap[0x002e] = Tmp(FORWARDL, 1);
	blockMap[0x088c] = Tmp(FORWARDL, 2);
	blockMap[0x00e8] = Tmp(FORWARDL, 3);
	blockMap[0x0c88] = Tmp(REVERSEL, 0);
	blockMap[0x00e2] = Tmp(REVERSEL, 1);
	blockMap[0x044c] = Tmp(REVERSEL, 2);
	blockMap[0x008e] = Tmp(REVERSEL, 3);
	blockMap[0x00c6] = Tmp(FORWARDZ, 0);
	blockMap[0x04c8] = Tmp(FORWARDZ, 1);
	blockMap[0x006c] = Tmp(REVERSEZ, 0);
	blockMap[0x08c4] = Tmp(REVERSEZ, 1);
	blockMap[0x00e4] = Tmp(FLATT, 0);
	blockMap[0x04c4] = Tmp(FLATT, 1);
	blockMap[0x004e] = Tmp(FLATT, 2);
	blockMap[0x08c8] = Tmp(FLATT, 3);
	lastOp = chrono::steady_clock::now();
	qDebug() << "Parameters Setting finished!";
	cv::setMouseCallback(WIN_NAME, mouseCallback, (void*)&(argM));
}

//鏋愭瀯
usrGameController::~usrGameController()
{
	cv::destroyAllWindows();
	if (device != nullptr)
	{
		delete device;
	}
	qDebug() << "usrGameController offline.";
}

void usrGameController::click(ButtonType bt, bool moveOnly) {
	switch (bt) {
	case STARTB:
		device->comMoveToScale(((double)start_button[1]) / pt_rows, 1 - ((double)start_button[0]) / pt_cols);
		qDebug() << "Start Game!";
		break;
	case LEFTB:
		device->comMoveToScale(((double)buttons[LEFTB].y) / pt_rows, 1 - ((double)buttons[LEFTB].x) / pt_cols);
		qDebug() << "Left!";
		break;
	case RIGHTB:
		device->comMoveToScale(((double)buttons[RIGHTB].y) / pt_rows, 1 - ((double)buttons[RIGHTB].x) / pt_cols);
		qDebug() << "Right!";
		break;
	case ROTATEB:
		device->comMoveToScale(((double)buttons[ROTATEB].y) / pt_rows, 1 - ((double)buttons[ROTATEB].x) / pt_cols);
		qDebug() << "Rotate!";
		break;
	case DROPB:
		device->comMoveToScale(((double)buttons[DROPB].y) / pt_rows, 1 - ((double)buttons[DROPB].x) / pt_cols);
		qDebug() << "Drop!";
		break;
	}
	if (!moveOnly) {
		device->comHitOnce();
		lastOp = chrono::steady_clock::now();
	}
}

void usrGameController::getStartButton(cv::Mat& img) {
	int thresh = 150;
	Mat canny_output, channel[3];
	vector<Mat> contours(300);
	split(img, channel);
	Canny(channel[1], canny_output, thresh, thresh * 2, 3);

	/// 瀵绘壘杞粨
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

	sort(sss.begin(), sss.end(), comp);

	///  璁＄畻涓績鐭�:
	vector<Point2f> mc(len);
	for (int i = 0; i < len; i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	int index = sss[0].i;
	start_button[0] = int(round(mc[index].x));
	start_button[1] = int(round(mc[index].y));
	qDebug() << start_button[0] << start_button[1];
	index = sss[1].i;
	if (int(round(mc[index].y)) < start_button[1]) {
		start_button[0] = int(round(mc[index].x));
		start_button[1] = int(round(mc[index].y));
	}
	qDebug() << start_button[0] << start_button[1];
	//cv::namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	//cv::imshow("Detection", channel[1]);
}

bool usrGameController::isMenu(cv::Mat& img) {
	int col = img.cols;
	int row = img.rows;
	double distance = 0;
	Mat mid = img(Rect(col / 4, row / 4, col / 2, row / 2));
	vector<double> feature = getFeature(mid);
	for (int i = 0; i < feature.size(); i++) {
		//qDebug() << menuFeature[i] << ' ' << feature[i];
		distance += pow(menuFeature[i] - feature[i], 2);
	}
	//qDebug() << distance;
	return distance < 1e-3;
}

void usrGameController::initialLocation(cv::Mat& img) {
	int thresh = 100;
	Mat canny_output,img_gray;
	vector<Mat> contours(500);
	vector<int> stop_button(2, 0);

	cvtColor(img, img_gray, CV_RGB2GRAY);
	/// 鐢–anny绠楀瓙妫�娴嬭竟缂�
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

	/// 瀵绘壘杞粨
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

	sort(sss.begin(), sss.end(), comp);

	///  璁＄畻涓績鐭�:
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

	vector<int> x;
	vector<int> y;
	vector<cv::Point> tmp(4);
	bool flag = false;
	int tmp_x, tmp_y, index;
	radius = 5;
	for (int num = 1; num < contours.size(); ++num){
		index = sss[num].i;
		tmp_x = int(round(mc[index].x));
		tmp_y = int(round(mc[index].y));
		if (tmp_y < background[UP] && tmp_x <= background[LEFT]) {
			if (stop_button[0] == 0) stop_button[0] = tmp_x;
			if (stop_button[1] == 0) stop_button[1] = tmp_y;
		}
		if (x.size() + y.size() < 4){
			if (tmp_y >= background[UP] && tmp_y <= background[DOWN]) {
				if (tmp_x <= background[LEFT] || tmp_x >= background[RIGHT]){
					if (x.size() == 0 && y.size() == 0){
						x.push_back(tmp_x);
						y.push_back(tmp_y);
						radius = sqrtf(sss[num].a / CV_PI);
						continue;
					}
					if (abs(tmp_x - x[0]) > background[WIDTH] && x.size() < 2) {
						x.push_back(tmp_x);
						flag = true;
					}
					int n = 0;
					for (; n < y.size(); ++n){
						if (abs(tmp_y - y[n]) < radius){
							break;
						}
					}
					if (n >= y.size()){
						y.push_back(tmp_y);
					}
				}
			}
		}
		if (stop_button[0] && stop_button[1]) break;
	}
	if (flag){
		tmp[0] = Point(x[0], y[0]); tmp[1] = Point(x[1], y[0]);
		tmp[2] = Point(x[0], y[1]); tmp[3] = Point(x[1], y[1]);
	}
	else {
		int dy = 0;
		sort(y, compp);
		dy = min(y[1] - y[0], y[2] - y[1]);
		if (y[2] + dy < background[DOWN]) y.push_back(y[2] + dy);
		else{
			if (y[2] - y[1] > dy * 3 / 2) y.push_back(y[1] + dy);
			else if (y[1] - y[0] > dy * 3 / 2) y.push_back(y[0] + dy);
			else y.push_back(y[0] - dy);
		}
		tmp[0] = Point(x[0], y[0]); tmp[1] = Point(x[0], y[1]);
		tmp[2] = Point(x[0], y[2]); tmp[3] = Point(x[0], y[3]);
	}

	Mat mv, mvt, target;
	stringstream ss;
	float dist, min_dist;
	int type;
	for (int i = 0; i < 4; ++i){
		mv = img(Rect(tmp[i].x - round(radius) / 2, tmp[i].y - round(radius) / 2, round(radius), round(radius)));
		resize(mv, mvt, Size(5, 5));
		type = 1;
		target = imread("../images/button/1.jpg");
		//cout << target.size() << endl;
		min_dist = norm(mvt, target, CV_L1);
		//cout << min_dist << endl;
		for (int j = 2; j <= 4; ++j){
			ss.str("");
			ss << j;
			target = imread("../images/button/" + ss.str() + ".jpg");
			//cout << target.size() << endl;
			dist = norm(mvt, target, CV_L1);
			if (dist < min_dist){
				type = j;
				min_dist = dist;
			}
		}
		buttons[type] = tmp[i];
		//cout << min_dist << '\t' << (ButtonType)type << '\t' << tmp[i] << endl;
	}

	double block_x = double(background[WIDTH]) / 10;
	double block_y = double(background[HEIGHT]) / 20;
	next_background[LEFT] = background[RIGHT] - round(block_x);
	next_background[RIGHT] = background[RIGHT] + round(3 * block_x);
	next_background[UP] = stop_button[1] - round(1.5*block_y);
	next_background[DOWN] = stop_button[1] + round(1.5*block_y);
	next_background[WIDTH] = next_background[RIGHT] - next_background[LEFT];
	next_background[HEIGHT] = next_background[DOWN] - next_background[UP];
	isInitialed = true;
}

vector<double> usrGameController::getFeature(cv::Mat& img) {
	vector<double> result_feature;
	cv::Mat integral_img;
	cv::integral(img, integral_img, CV_32F);
	cv::Vec3f sub[4];
	int col = img.cols;
	int row = img.rows;
	sub[0] = integral_img.at<cv::Vec3f>(row / 2, col / 2);
	sub[1] = integral_img.at<cv::Vec3f>(row , col / 2) - integral_img.at<cv::Vec3f>(row / 2, col / 2);
	sub[2] = integral_img.at<cv::Vec3f>(row / 2, col) - integral_img.at<cv::Vec3f>(row / 2, col / 2);
	sub[3] = integral_img.at<cv::Vec3f>(row, col) - integral_img.at<cv::Vec3f>(row, col / 2) - integral_img.at<cv::Vec3f>(row / 2, col) + integral_img.at<cv::Vec3f>(row / 2, col / 2);
	for (int i = 0; i < 4; i++) {
		double r = sub[i][0];
		double g = sub[i][1];
		double b = sub[i][2];
		double sum = r + g + b;
		//qDebug() << r << ' ' << g << ' ' << b << ' ' << sum;
		result_feature.push_back(r / sum);
		result_feature.push_back(g / sum);
		result_feature.push_back(b / sum);
	}
	for (int j = 0; j < result_feature.size(); j++) {
		//qDebug() << result_feature[j];
	}
	return result_feature;
}

BlockType usrGameController::getBlockType(cv::Mat& img) {
	vector<double> features = getFeature(img);
	//qDebug() << "features";
	int type;
	double distance, max_distance = 100;
	for (int i = 0; i < 7; i++) {
		distance = 0;
		for (int j = 0; j < features.size(); j++) {
			distance += pow(blockFeatures[i][j] - features[j],2);
		}
		//qDebug() << "type:" << i << " distance:" << distance;
		if (distance < max_distance) {
			type = i;
			max_distance = distance;
		}
	}
	if (max_distance > 1e-2) type = 7;
	return BlockType(type);
}

usrGameController::Tmp usrGameController::readBlockFromMatrix(vector<vector<int>> grid, bool isFirst) {
	int up, left;
	bool flag = true;
	for (int i = 0; i < 2; i++) {
		flag = (accumulate(grid[i].cbegin(), grid[i].cend(), 0) == 0) & flag;
	}

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 10; j++) grid[i][j] -= currentState.getState(i, j);
	}
	if (isFirst) {
		up = 19;
		for (left = 0; left < 10; left++) {
			if (grid[up][left] + grid[up - 1][left] + grid[up - 2][left] + grid[up - 3][left] > 0) break;
		}
	}
	else {
		for (up = 19; up > -1; up--) {
			if (accumulate(grid[up].cbegin(), grid[up].cend(), 0) > 0) break;
		}
		//qDebug() << up;
		for (left = 0; left < 10; left++) {
			int c = grid[max(up, 0)][left] + grid[max(up - 1, 0)][left] + grid[max(up - 2, 0)][left] + grid[max(up - 3, 0)][left];
			if (c > 0) break;
		}
	}
	int index = 0;
	for (int i = up - 3; i <= up; i++) for (int j = left; j < left + 4; j++) {
		index *= 2;
		if(i >= 0 && j < 10) index += grid[i][j];
	}
	if (blockMap.find(index) == blockMap.end()) {
		for (int i = up - 3; i <= up; i++) {
			if (left + 3 < 10 && i >= 0) qDebug() << grid[i][left] << grid[i][left + 1] << grid[i][left + 2] << grid[i][left + 3];
			else if (left == 7 && i >= 0) qDebug() << grid[i][left] << grid[i][left + 1] << grid[i][left + 2];
			else if (left == 8 && i >= 0) qDebug() << grid[i][left] << grid[i][left + 1];
			else if (left == 9 && i >= 0) qDebug() << grid[i][left];
		}
		return Tmp(SLIDE, 4, left, flag);
	} else {
		
		Tmp res = blockMap[index];
		res.loc = left;
		res.clear = flag;
		//qDebug() << "Block Type: " << res.t;
		return res;
	}
}

vector<vector<int>> usrGameController::readFromImg(cv::Mat& img, bool isFirst) {
	int erosion_size;
	vector<vector<int>> result(20, vector<int>(10, 0));
	if (isFirst) {
		//qDebug() << "Find first block";
		Mat dst,resize_dst;
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
		threshold(dst, dst, 80, 255, 0);
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

//澶勭悊鍥惧儚 
int usrGameController::usrProcessImage(cv::Mat& img)
{
	
	cv::Size imgSize(img.cols, img.rows - UP_CUT);
	if (imgSize.height <= 0 || imgSize.width <= 0)
	{
		qDebug() << "Invalid image. Size:" << imgSize.width <<"x"<<imgSize.height;
		return -1;
	}

	//鎴彇鍥惧儚杈圭紭
	cv::Mat rgba_pt = img(cv::Rect(0, UP_CUT, imgSize.width,imgSize.height));
	cv::Mat pt, main_area, tmp;
	vector<Mat> hsv_model;
	cv::cvtColor(rgba_pt, pt, CV_RGBA2RGB);
	cv::imshow(WIN_NAME, pt);

	if (pt_cols == 0) pt_cols = pt.cols;
	if (pt_rows == 0) pt_rows = pt.rows;

	BlockType nextType;
	vector<vector<int>> grid,new_grid;
	Tmp block_tmp;
	chrono::steady_clock::time_point current = chrono::steady_clock::now();

	if (isGame) {
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2HSV);
		split(main_area, hsv_model);
		main_area = hsv_model[2];
		grid = readFromImg(main_area);
		combineAndshow(pt, main_area);
		showState(pt);
	}
	if (currentRecursiveState == GAMEOVER) {
		destroyWindow("ComputerVision");
		destroyWindow("State");
	}

	if(chrono::duration_cast<chrono::milliseconds>(current - lastOp).count() > Op_delay) switch (currentRecursiveState) {
	case MENU:
		qDebug() << "Menu...";
		if (start_button[0] == 0) {
			//get start button
			getStartButton(pt);
		}
		click(STARTB);
		Op_delay = 1500;
		currentRecursiveState = INITIAL;
		break;
	case INITIAL:
		if (isMenu(pt)) {
			qDebug() << "Start Fail! Return...";
			currentRecursiveState = MENU;
			break;
		}
		Op_delay = 950;
		qDebug() << "Initializing...";
		if(background[WIDTH] == 0) initialLocation(pt);
		currentState.clear();
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2HSV);
		split(main_area, hsv_model);
		main_area = hsv_model[2];
		grid = readFromImg(main_area, true);
		block_tmp = readBlockFromMatrix(grid, true);
		nextBlock = gameBlock(block_tmp.t);
		currentRecursiveState = NEWCYCLE;
		isGame = true;
		qDebug() << " Start...";
		break;
	case NEWCYCLE:
		delay_count = 0;
		fail_count = 0;
		check_count = 0;
		jump_flag = false;
		waitUp = false;
		currentRecursiveState = NEXTBLOCK;
		qDebug() << "Judging Next Block...";
		break;

	case NEXTBLOCK:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		currentBlock = nextBlock;
		nextBlock = gameBlock(nextType);
		currentRecursiveState = DELAY;	
		break;
	case DELAY:
		if (++delay_count > DELAY_STEP) {
			currentRecursiveState = REFRESH;
			qDebug() << "Refresh State...";
		}
		break;
	case REFRESH:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		currentState.refresh(grid);
		currentState.show();
		currentRecursiveState = FINDLOC;
		break;
	case FINDLOC:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		qDebug() << "Finding Best Location...";
		//click(ROTATEB, true);
		qDebug() << "current block type:";
		currentBlock.show();
		qDebug() << "next block type";
		nextBlock.show();
		currentBestLoc = currentState.getBestDropLoc(currentBlock/*, nextBlock*/);
		//if (currentBestLoc.del_row != 0) doubleDetection = true;
		qDebug() << "Best loc: " << currentBestLoc.loc;
		qDebug() << "Best rotation" << currentBestLoc.rotation;
		currentRecursiveState = JUDGELOC;
		qDebug() << "Judging Location...";
		break;
	case JUDGELOC:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		if (jump_flag && accumulate(grid[0].cbegin(), grid[0].cend(), 0) > 0 && accumulate(grid[1].cbegin(), grid[1].cend(), 0) == 0) {
			currentRecursiveState = NEWCYCLE;
			qDebug() << "Next Step...";
			break;
		}
		block_tmp = readBlockFromMatrix(grid);

		if (block_tmp.clear) jump_flag = true;

		if (block_tmp.r == 4) {
			cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), tmp, CV_RGB2HSV);
			split(tmp, hsv_model);
			tmp = hsv_model[2];
			new_grid = readFromImg(tmp, true);
			block_tmp = readBlockFromMatrix(new_grid);
		}
		qDebug() << "Rotation: " << block_tmp.r << '(' << currentBestLoc.rotation << ')';
		qDebug() << "Loc: " << block_tmp.loc << '(' << currentBestLoc.loc << ')';

		if (block_tmp.loc == currentBestLoc.loc && block_tmp.r == currentBestLoc.rotation) {
			currentRecursiveState = CHECK;
			qDebug() << "Check...";
		}
		else {
			if (block_tmp.r != 4 && block_tmp.r != currentBestLoc.rotation) {
				click(ROTATEB);
			}
			else if (block_tmp.loc != currentBestLoc.loc){
				if (block_tmp.loc < currentBestLoc.loc) click(RIGHTB);
				else if (block_tmp.loc > currentBestLoc.loc) click(LEFTB);
			}
			else if(jump_flag && block_tmp.r == 4){
				if (++fail_count > FAIL_STEP) {
					currentRecursiveState = DROP;
					qDebug() << "Reading Block Fail...";
					qDebug() << "Dropping...";
				}
			}
		}
		break;
	case CHECK:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		block_tmp = readBlockFromMatrix(grid);
		if (block_tmp.r == 4) {
			cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), tmp, CV_RGB2HSV);
			split(tmp, hsv_model);
			tmp = hsv_model[2];
			new_grid = readFromImg(tmp, true);
			block_tmp = readBlockFromMatrix(new_grid);
		}
		if (jump_flag && accumulate(grid[0].cbegin(), grid[0].cend(), 0) > 0 && accumulate(grid[1].cbegin(), grid[1].cend(), 0) == 0) {
			currentRecursiveState = NEWCYCLE;
			qDebug() << "Next Step...";
			break;
		}
		if (block_tmp.loc == currentBestLoc.loc && block_tmp.r == currentBestLoc.rotation) {
			if (++check_count > CHECK_STEP) {
				currentRecursiveState = DROP;
				qDebug() << "Dropping...";
			}
		}
		else {
			currentRecursiveState = JUDGELOC;
			qDebug() << "DoubleCheck Failed!";
		}
		break;
	case DROP:
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (nextType == NONE) {
			lastState = currentRecursiveState;
			currentRecursiveState = GAMEOVER;
			break;
		}
		if (accumulate(grid[0].cbegin(), grid[0].cend(), 0) > 0 && accumulate(grid[1].cbegin(), grid[1].cend(), 0) == 0) {
			device->comHitUp();
			waitUp = false;
			currentRecursiveState = NEWCYCLE;
			qDebug() << "Next Step...";
			
		}
		else {
			
			if (!waitUp) {
				click(DROPB, true);
				device->comHitDown();
			}
			waitUp = true;
		}
		break;
	case GAMEOVER:
		qDebug() << "Game Over";
		device->comHitUp();
		device->comMoveToScale(0, 0);
		//currentState.clear();
		currentRecursiveState = NEWGAME;
		isGame = false;
		break;
	case NEWGAME:
		if (isMenu(pt)) {
			qDebug() << "Start New Game";
			currentRecursiveState = MENU;
		}
		else if (isInitialed) { //防止广告
			nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
			if (nextType != NONE) {
				qDebug() << "Reconnect to Game";
				isGame = true;
				cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), tmp, CV_RGB2HSV);
				split(tmp, hsv_model);
				tmp = hsv_model[2];
				new_grid = readFromImg(tmp);
				if (accumulate(new_grid[19].begin(), new_grid[19].end(), 0) == 0) {
					currentRecursiveState = INITIAL;
				}
				else currentRecursiveState = lastState;
			}
		}
		lastOp = chrono::steady_clock::now();
		break;
	
	}
	else if (isGame && currentRecursiveState != NEWCYCLE) {
		if (jump_flag && accumulate(grid[0].cbegin(), grid[0].cend(), 0) > 0 && accumulate(grid[1].cbegin(), grid[1].cend(), 0) == 0) {
			currentRecursiveState = NEWCYCLE;
			qDebug() << "Next Step...";
		}
	}
	savePicture(pt);
	return 0; 
}

/*for test*/
void mouseCallback(int event, int x, int y, int flags, void*param)
{
	usrGameController::MouseArgs* m_arg = (usrGameController::MouseArgs*)param;
	switch (event)
	{
	case CV_EVENT_LBUTTONUP:case CV_EVENT_RBUTTONUP: // 宸�/鍙抽敭寮硅捣
	{
		m_arg->Hit = true;
	}
	break;
	}
}

void usrGameController::showDetection(cv::Mat& img) {
	cv::circle(img, buttons[LEFTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[RIGHTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[DROPB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[ROTATEB], 5, Scalar(0), 2, 8, 0);
	cv::rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::rectangle(img, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	cv::imshow("Detection", img);
}

void usrGameController::combineAndshow(cv::Mat& pt, cv::Mat& main_area) {
	Mat dst,imgg;
	pt.copyTo(imgg);
	int col = background[WIDTH];
	int row = background[HEIGHT];
	for (int i = 1; i < 10; i++) {
		line(main_area, Point(i*col / 10, 0), Point(i*col / 10, row - 1), Scalar(255), 1, 8, 0);
	}
	for (int i = 1; i < 20; i++) {
		line(main_area, Point(0, i*row / 20), Point(col - 1, i*row / 20), Scalar(255), 1, 8, 0);
	}
	cvtColor(main_area, dst, CV_GRAY2RGB);
	dst.copyTo(imgg(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])));

	cv::circle(imgg, buttons[LEFTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(imgg, buttons[RIGHTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(imgg, buttons[DROPB], 5, Scalar(0), 2, 8, 0);
	cv::circle(imgg, buttons[ROTATEB], 5, Scalar(0), 2, 8, 0);
	//rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::rectangle(imgg, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::namedWindow("ComputerVision", CV_WINDOW_AUTOSIZE);
	cv::imshow("ComputerVision", imgg);
}

void usrGameController::showState(cv::Mat& pt) {
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

	cv::circle(img, buttons[LEFTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[RIGHTB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[DROPB], 5, Scalar(0), 2, 8, 0);
	cv::circle(img, buttons[ROTATEB], 5, Scalar(0), 2, 8, 0);
	//rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::rectangle(img, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);
	cv::namedWindow("State", CV_WINDOW_AUTOSIZE);
	cv::imshow("State", img);
}

void usrGameController::savePicture(cv::Mat& img) {
	if (argM.Hit) {
		struct tm *p;
		time_t current_time;
		time(&current_time);
		p = localtime(&current_time);
		stringstream ss;
		ss << (p->tm_year + 1900) << (p->tm_mon + 1) << p->tm_mday << p->tm_hour << p->tm_min << p->tm_sec;
		string name;
		name = ss.str();
		cv::Size imgSize(img.cols, img.rows);
		cv::imwrite(("../images/"+name + ".jpg").c_str(), img);
		qDebug() << "Picture Saved!";
		argM.Hit = false;
	}
}
/*end for test*/
#endif
