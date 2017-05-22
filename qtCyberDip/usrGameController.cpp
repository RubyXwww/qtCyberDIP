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
	for (int i = 0; i < 7; i++) {
		vector<double> feature;
		for (int j = 0; j < 12; j++) {
			fin >> tmp;
			feature.push_back(tmp);
		}
		blockFeatures.push_back(feature);
	}
	fin.close();

	background = vector<int>(6, 0);
	next_background = vector<int>(6, 0);
	buttons = vector<int>(4, 0);
	isFirstBlock = false;
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

void usrGameController::initialLocation(cv::Mat& img) {
	int thresh = 100;
	Mat canny_output,img_gray;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	vector<int> stop_button(2, 0);

	cvtColor(img, img_gray, CV_RGB2GRAY);
	/// 用Canny算子检测边缘
	Canny(img_gray, canny_output, thresh, thresh * 2, 3);

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

	for (int n = 0; n < 4; ++n) {
		buttons[n] = 0;
	}

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
	qDebug() << "Mat Type:" << img.channels();
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
		qDebug() << r << ' ' << g << ' ' << b << ' ' << sum;
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
	qDebug() << "features";
	int type;
	double distance, max_distance = 100;
	for (int i = 0; i < 7; i++) {
		distance = 0;
		for (int j = 0; j < features.size(); j++) {
			distance += pow(blockFeatures[i][j] - features[j],2);
		}
		qDebug() << "type:" << i << " distance:" << distance;
		if (distance < max_distance) {
			type = i;
			max_distance = distance;
		}
	}
	return BlockType(type);
}

//处理图像 
int usrGameController::usrProcessImage(cv::Mat& img)
{
	struct tm *p;
	time_t current_time;
	time(&current_time);
	p = localtime(&current_time);
	stringstream ss;
	ss << (p->tm_year + 1900) << (p->tm_mon + 1) << p->tm_mday << p->tm_hour << p->tm_min << p->tm_sec;
	string name;
	name = ss.str();

	cv::Size imgSize(img.cols, img.rows - UP_CUT);
	if (imgSize.height <= 0 || imgSize.width <= 0)
	{
		qDebug() << "Invalid image. Size:" << imgSize.width <<"x"<<imgSize.height;
		return -1;
	}

	//截取图像边缘
	cv::Mat rgba_pt = img(cv::Rect(0, UP_CUT, imgSize.width,imgSize.height));
	cv::Mat pt;
	cv::cvtColor(rgba_pt, pt, CV_RGBA2RGB);
	cv::imshow(WIN_NAME, pt);
	BlockType nextType;
	switch (currentRecursiveState) {
	case MENU:
		qDebug() << "Menu...";
		break;
	case INITIAL:
		qDebug() << "Initializing...";
		initialLocation(pt);
		isFirstBlock = true;
		currentRecursiveState = NEXTBLOCK;
		break;
	case NEXTBLOCK:
		qDebug() << "Judging Next Block...";
		nextType = getBlockType(pt(Rect(next_background[LEFT], next_background[UP], next_background[WIDTH], next_background[HEIGHT])));
		if (!isFirstBlock) currentBlock = nextBlock;
		nextBlock = gameBlock(nextType);
		qDebug() << "BlockType:" << int(nextType);
		//currentRecursiveState = FINDLOC;
		break;
	case FINDLOC:
		qDebug() << "Finding Best Location...";
		break;
	case JUDGELOC:
		qDebug() << "Judging Location...";
		break;
	case DROP:
		qDebug() << "Dropping...";
		break;
	}

	if (currentRecursiveState != NEXTBLOCK | currentRecursiveState != MENU) showDetection(pt);
	
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
		argM.box.x = -1; argM.box.y = -1;
		if (argM.Hit)
		{
			device->comHitUp();
		}
		else
		{
			device->comHitOnce();
		}
	}
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