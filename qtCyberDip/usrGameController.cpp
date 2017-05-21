#include "usrGameController.h"
#include <time.h>
#include <sstream>
#include<fstream>
#ifdef VIA_OPENCV
//构造与初始化
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
		cv::Mat block_img = pt(cv::Rect(370, 30, 95, 75));
		BlockType type = getBlockType(block_img);
		//cv::imwrite(("../images/cap_block/" + name + ".jpg").c_str(), block_img);
		qDebug() << "BlockType:" << int(type);
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
#endif