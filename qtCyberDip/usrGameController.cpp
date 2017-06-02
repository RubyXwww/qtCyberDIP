#include "usrGameController.h"
#include <time.h>
#include <sstream>
#include<fstream>
#ifdef VIA_OPENCV
//构造与初始化
using namespace cv;

struct ss {
	int i; float a;
};

bool comp(const ss &a, const ss &b)
{
	return a.a>b.a;
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
	device = new deviceCyberDip(qtCD);//设备代理类
	cv::namedWindow(WIN_NAME);
	/*
	ofstream out;
	out.open("../images/blocklabel/features.txt");
	for (int i = 0; i < 7; i++) {
		stringstream ss;
		ss << i;
		cv::Mat img = cv::imread(("../images/blocklabel/" + ss.str() + "_.jpg").c_str());
		for (double f : getFeature(img)) {
			out << f << '\t';
		}
		out << '\n';
	}
	out.close();*/
	
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
	buttons = vector<int>(4, 0);
	start_button = vector<int>(2, 0);
	isFirstBlock = false;
	currentRecursiveState = MENU;

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

//析构
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
		device->comMoveToScale(((double)buttons[DOWN]) / pt_rows, 1 - ((double)buttons[LEFT]) / pt_cols);
		qDebug() << "Left!";
		break;
	case RIGHTB:
		device->comMoveToScale(((double)buttons[DOWN]) / pt_rows, 1 - ((double)buttons[RIGHT]) / pt_cols);
		qDebug() << "Right!";
		break;
	case ROTATEB:
		device->comMoveToScale(((double)buttons[UP]) / pt_rows, 1 - ((double)buttons[RIGHT]) / pt_cols);
		qDebug() << "Rotate!";
		break;
	case DROPB:
		device->comMoveToScale(((double)buttons[UP]) / pt_rows, 1 - ((double)buttons[LEFT]) / pt_cols);
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

	/// 寻找轮廓
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

	///  计算中心矩:
	vector<Point2f> mc(len);
	for (int i = 0; i < len; i++)
	{
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
	}

	int index = sss[0].i;
	start_button[0] = int(round(mc[index].x));
	start_button[1] = int(round(mc[index].y));
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
	vector<Mat> contours(300);
	vector<int> stop_button(2, 0);

	cvtColor(img, img_gray, CV_RGB2GRAY);
	/// 用Canny算子检测边缘
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

	/// 寻找轮廓
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

	///  计算中心矩:
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

	int tmp_x, tmp_y, index;
	for (int num = 1; num < contours.size(); ++num) {
		index = sss[num].i;
		tmp_x = int(round(mc[index].x));
		tmp_y = int(round(mc[index].y));
		if (tmp_y < background[UP] && tmp_x <= background[LEFT]) {
			if (stop_button[0] == 0) stop_button[0] = tmp_x;
			if (stop_button[1] == 0) stop_button[1] = tmp_y;
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
		if (buttons[LEFT] && buttons[RIGHT] && buttons[UP] && buttons[DOWN] && stop_button[0] && stop_button[1]) break;
	}

	swap(buttons[LEFT], buttons[RIGHT]);
	swap(buttons[UP], buttons[DOWN]);

	double block_x = double(background[WIDTH]) / 10;
	double block_y = double(background[HEIGHT]) / 20;
	next_background[LEFT] = background[RIGHT] - round(block_x);
	next_background[RIGHT] = background[RIGHT] + round(3 * block_x);
	next_background[UP] = stop_button[1] - round(1.5*block_y);
	next_background[DOWN] = stop_button[1] + round(1.5*block_y);
	next_background[WIDTH] = next_background[RIGHT] - next_background[LEFT];
	next_background[HEIGHT] = next_background[DOWN] - next_background[UP];

	
	circle(img, Point(buttons[LEFT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[LEFT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[RIGHT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[RIGHT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	rectangle(img, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);
	namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	imshow("Detection", img);

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
	return BlockType(type);
}

usrGameController::Tmp usrGameController::readBlockFromMatrix(vector<vector<int>> grid, bool isFirst) {
	int up, left;

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
		qDebug() << up;
		for (left = 0; left < 10; left++) {
			int c = grid[up][left] + grid[max(up - 1, 0)][left] + grid[max(up - 2, 0)][left] + grid[max(up - 3, 0)][left];
			if (c > 0) break;
		}
	}
	int index = 0;
	for (int i = up - 3; i <= up; i++) for (int j = left; j < left + 4; j++) {
		index *= 2;
		if(i >= 0) index += grid[i][j];
	}
	if (blockMap.find(index) == blockMap.end()) return Tmp(SLIDE, 4, left);
	else {
		Tmp res = blockMap[index];
		res.loc = left;
		return res;
	}
}

vector<vector<int>> usrGameController::readFromImg(cv::Mat& img, bool isFirst) {
	int erosion_size;
	vector<vector<int>> result(20, vector<int>(10, 0));
	if (isFirst) {
		qDebug() << "Find first block";
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
		threshold(dst, dst, 30, 255, 0);
		resize(dst, resize_dst, Size(10, 20));
		threshold(resize_dst, resize_dst, 200, 255, 0);
		imshow("resize", resize_dst);
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 10; j++) {
				if ((int)resize_dst.at<uchar>(i, j) == 255) result[i][j] = 1;
			}
		}
	}
	return result;
	
}

//处理图像 
int usrGameController::usrProcessImage(cv::Mat& img)
{
	static int delay = 1000;

	cv::Size imgSize(img.cols, img.rows - UP_CUT);
	if (imgSize.height <= 0 || imgSize.width <= 0)
	{
		qDebug() << "Invalid image. Size:" << imgSize.width <<"x"<<imgSize.height;
		return -1;
	}

	//截取图像边缘
	cv::Mat rgba_pt = img(cv::Rect(0, UP_CUT, imgSize.width,imgSize.height));
	cv::Mat pt, main_area;
	cv::cvtColor(rgba_pt, pt, CV_RGBA2RGB);
	cv::imshow(WIN_NAME, pt);

	if (pt_cols == 0) pt_cols = pt.cols;
	if (pt_rows == 0) pt_rows = pt.rows;

	BlockType nextType;
	vector<vector<int>> grid;
	Tmp block_tmp;
	chrono::steady_clock::time_point current = chrono::steady_clock::now();
	if(chrono::duration_cast<chrono::milliseconds>(current - lastOp).count() > delay) switch (currentRecursiveState) {
	case MENU:
		qDebug() << "Menu...";
		if (start_button[0] == 0) {
			getStartButton(pt);
			qDebug() << "Start Button get!";
		}
		click(STARTB);
		delay = 1200;
		qDebug() << "Clicked!";
		currentRecursiveState = INITIAL;
		break;
	case INITIAL:
		if (isMenu(pt)) {
			qDebug() << "Start Fail! Return...";
			currentRecursiveState = MENU;
			break;
		}
		delay = 1000;
		qDebug() << "Initializing...";
		initialLocation(pt);
		//isFirstBlock = true;
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2GRAY);
		grid = readFromImg(main_area, true);
		block_tmp = readBlockFromMatrix(grid, true);
		nextBlock = gameBlock(block_tmp.t);
		currentState.clear();
		currentRecursiveState = NEXTBLOCK;
		break;
	case NEXTBLOCK:
		qDebug() << "Judging Next Block...";
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		currentBlock = nextBlock;
		nextBlock = gameBlock(nextType);
		qDebug() << "BlockType:" << int(nextType);
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2GRAY);
		grid = readFromImg(main_area);
		currentState.refresh(grid);
		currentState.show();
		currentRecursiveState = FINDLOC;
		break;
	case FINDLOC:
		qDebug() << "Finding Best Location...";
		click(ROTATEB, true);
		qDebug() << "current block type:";
		currentBlock.show();
		qDebug() << "next block type";
		nextBlock.show();
		currentBestLoc = currentState.getBestDropLoc(currentBlock/*, nextBlock*/);
		qDebug() << "Best loc: " << currentBestLoc.loc;
		qDebug() << "Best rotation" << currentBestLoc.rotation;
		currentRecursiveState = JUDGELOC;
		break;
	case JUDGELOC:
		qDebug() << "Judging Location...";
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2GRAY);
		grid = readFromImg(main_area);
		if (accumulate(grid[0].cbegin(), grid[0].cend(), 0) > 0) currentRecursiveState = NEXTBLOCK;
		else {
			block_tmp = readBlockFromMatrix(grid);
			qDebug() << "Rotation: " << block_tmp.r << '(' << currentBestLoc.rotation << ')';
			qDebug() << "Loc: " << block_tmp.loc << '(' << currentBestLoc.loc << ')';
			if (block_tmp.loc == currentBestLoc.loc && block_tmp.r == currentBestLoc.rotation) currentRecursiveState = DROP;
			else {
				if (block_tmp.r != 4 && block_tmp.r != currentBestLoc.rotation) {
					click(ROTATEB);
				}
				else {
					if (block_tmp.loc < currentBestLoc.loc) click(RIGHTB);
					else if (block_tmp.loc > currentBestLoc.loc) click(LEFTB);
				}
			}
		}
		break;
	case DROP:
		qDebug() << "Dropping...";
		cvtColor(pt(Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT])), main_area, CV_RGB2GRAY);
		grid = readFromImg(main_area);
		if(accumulate(grid[0].cbegin(),grid[0].cend(),0) > 0) currentRecursiveState = NEXTBLOCK;
		else click(DROPB);
		break;
	}

	if (currentRecursiveState != MENU && buttons[0] != 0) showDetection(pt);
	/*
	struct tm *p;
	time_t current_time;
	time(&current_time);
	p = localtime(&current_time);
	stringstream ss;
	ss << (p->tm_year + 1900) << (p->tm_mon + 1) << p->tm_mday << p->tm_hour << p->tm_min << p->tm_sec;
	string name;
	name = ss.str();


	//判断鼠标点击尺寸
	if (argM.box.x >= 0 && argM.box.x < imgSize.width&&
		argM.box.y >= 0 && argM.box.y < imgSize.height
		)
	{
		
		qDebug() << "X:" << argM.box.x << " Y:" << argM.box.y;
		qDebug() << pt.at<cv::Vec3b>(argM.box.x ,argM.box.y)[0] << ' ' << pt.at<cv::Vec3b>(argM.box.x, argM.box.y)[1] << ' ' << pt.at<cv::Vec3b>(argM.box.x, argM.box.y)[2];
		if (argM.Hit)
		{
			device->comHitDown();
		}
		device->comMoveToScale(((double)argM.box.y + argM.box.height) / pt.rows , 1 - ((double)argM.box.x + argM.box.width) / pt.cols);
		//cv::imwrite(("../images/"+name + ".jpg").c_str(), pt);
		//cv::Mat block_img = pt(cv::Rect(370, 30, 95, 75));
		//BlockType type = getBlockType(block_img);
		//cv::imwrite(("../images/cap_block/" + name + ".jpg").c_str(), block_img);
		//qDebug() << "BlockType:" << int(type);
		//
		currentRecursiveState = NEXTBLOCK;
		argM.box.x = -1; argM.box.y = -1;
		if (argM.Hit)
		{
			device->comHitUp();
		}
		else
		{
			device->comHitOnce();
		}
	}*/
	return 0; 
}

//鼠标回调函数
void mouseCallback(int event, int x, int y, int flags, void*param)
{
	usrGameController::MouseArgs* m_arg = (usrGameController::MouseArgs*)param;
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE: // 鼠标移动时
	{
		if (m_arg->Drawing)
		{
			m_arg->box.width = x - m_arg->box.x;
			m_arg->box.height = y - m_arg->box.y;
		}
	}
	break;
	case CV_EVENT_LBUTTONDOWN:case CV_EVENT_RBUTTONDOWN: // 左/右键按下
	{
		m_arg->Hit = event == CV_EVENT_RBUTTONDOWN;
		m_arg->Drawing = true;
		m_arg->box = cvRect(x, y, 0, 0);
	}
	break;
	case CV_EVENT_LBUTTONUP:case CV_EVENT_RBUTTONUP: // 左/右键弹起
	{
		m_arg->Hit = false;
		m_arg->Drawing = false;
		if (m_arg->box.width < 0)
		{
			m_arg->box.x += m_arg->box.width;
			m_arg->box.width *= -1;
		}
		if (m_arg->box.height < 0)
		{
			m_arg->box.y += m_arg->box.height;
			m_arg->box.height *= -1;
		}
	}
	break;
	}
}

/*for test*/
void usrGameController::showDetection(cv::Mat& img) {
	circle(img, Point(buttons[LEFT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[LEFT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[RIGHT], buttons[DOWN]), 5, Scalar(0), 2, 8, 0);
	circle(img, Point(buttons[RIGHT], buttons[UP]), 5, Scalar(0), 2, 8, 0);
	rectangle(img, Rect(background[LEFT], background[UP], background[WIDTH], background[HEIGHT]), Scalar(0), 2, 8, 0);
	rectangle(img, Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT]), Scalar(0), 2, 8, 0);
	namedWindow("Detection", CV_WINDOW_AUTOSIZE);
	imshow("Detection", img);
}
/*end for test*/
#endif