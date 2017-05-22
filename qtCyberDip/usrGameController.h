#ifdef VIA_OPENCV

#ifndef USRGAMECONTROLLER_H
#define USRGAMECONTROLLER_H

#include "qtcyberdip.h"
#include "gameState.h"
#define WIN_NAME "Frame"

//游戏控制类
class usrGameController
{
private:
	enum BackgroundInformation {
		UP, DOWN, LEFT, RIGHT, WIDTH, HEIGHT
	};
	enum RecursiveState {
		MENU,
		INITIAL,
		NEXTBLOCK,
		FINDLOC,
		JUDGELOC,
		DROP
	};
	RecursiveState currentRecursiveState;
	deviceCyberDip* device;
	vector<vector<double>> blockFeatures;
	vector<int> background;
	vector<int> next_background;
	vector<int> buttons;
	gameBlock currentBlock;
	gameBlock nextBlock;
	bool isFirstBlock;
	
//以下是为了实现演示效果，增加的内容
	//鼠标回调结构体
	struct MouseArgs{
		cv::Rect box;
		bool Drawing, Hit;
		// init
		MouseArgs() :Drawing(false), Hit(false)
		{
			box = cv::Rect(0, 0, -1, -1);
		}
	};
	//鼠标回调函数
	friend void  mouseCallback(int event, int x, int y, int flags, void*param);
	MouseArgs argM;
//以上是为了实现课堂演示效果，增加的内容
	vector<double> getFeature(cv::Mat& img);
	void initialLocation(cv::Mat& img);
	/*for test*/
	void showDetection(cv::Mat& img);
	/*end for test*/
	
public:
	//构造函数，所有变量的初始化都应在此完成
	usrGameController(void* qtCD);
	//析构函数，回收本类所有资源
	~usrGameController();
	//处理图像函数，每次收到图像时都会调用
	BlockType getBlockType(cv::Mat& img);
	int usrProcessImage(cv::Mat& img);
};

//以下是为了实现演示效果，增加的内容
//鼠标回调函数
void  mouseCallback(int event, int x, int y, int flags, void*param);
//以上是为了实现课堂演示效果，增加的内容

#endif
#endif