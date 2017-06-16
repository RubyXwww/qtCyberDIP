#ifdef VIA_OPENCV

#ifndef USRGAMECONTROLLER_H
#define USRGAMECONTROLLER_H

#include "qtcyberdip.h"
#include "gameState.h"
#include <map>
#include <chrono>
#define WIN_NAME "Frame"

//游戏控制类
class usrGameController
{
private:
	struct Tmp {
		BlockType t;
		int r;
		int loc;
		bool clear;
		Tmp(BlockType tp=SLIDE, int rr = 0, int x=0, bool c=false):t(tp),r(rr),loc(x),clear(c) {}
	};

	enum BackgroundInformation {
		UP, DOWN, LEFT, RIGHT, WIDTH, HEIGHT
	};
	enum RecursiveState {
		MENU,
		INITIAL,
		NEWCYCLE,
		NEXTBLOCK,
		DELAY,
		REFRESH,
		FINDLOC,
		JUDGELOC,
		CHECK,
		DROP,
		GAMEOVER,
		NEWGAME
	};
	enum ButtonType {
		STARTB,
		LEFTB,
		RIGHTB,
		ROTATEB,
		DROPB
	};
	RecursiveState currentRecursiveState;
	RecursiveState lastState;
	deviceCyberDip* device;
	vector<double> menuFeature;
	vector<vector<double>> blockFeatures;
	vector<int> background;
	vector<int> next_background;
	//vector<int> buttons;
	vector<cv::Point> buttons;
	vector<int> start_button;
	map<int, Tmp> blockMap;
	int pt_cols, pt_rows;
	gameBlock currentBlock;
	gameBlock nextBlock;
	gameState currentState;
	DropLoc currentBestLoc;
	bool isFirstBlock;
	bool jump_flag;
	bool isGame;
	bool isInitialed;
	bool waitUp;
	int delay_count;
	int fail_count;
	int check_count;
	int Op_delay;
	chrono::steady_clock::time_point lastOp = chrono::steady_clock::now();
	
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
	void getStartButton(cv::Mat& img);
	bool isMenu(cv::Mat& img);
	void initialLocation(cv::Mat& img);
	void click(ButtonType bt, bool moveOnly = false);
	BlockType getBlockType(cv::Mat& img);
	vector<vector<int>> readFromImg(cv::Mat& img, bool isFirst = false);
	Tmp readBlockFromMatrix(vector<vector<int>> grid, bool isFirst = false);
	/*for test*/
	void showDetection(cv::Mat& img);
	void combineAndshow(cv::Mat& img, cv::Mat& main_area);
	void showState(cv::Mat& img);
	void savePicture(cv::Mat& img);
	/*end for test*/
	
public:
	//构造函数，所有变量的初始化都应在此完成
	usrGameController(void* qtCD);
	//析构函数，回收本类所有资源
	~usrGameController();
	
	int usrProcessImage(cv::Mat& img);
};

//以下是为了实现演示效果，增加的内容
//鼠标回调函数
void  mouseCallback(int event, int x, int y, int flags, void*param);
//以上是为了实现课堂演示效果，增加的内容

#endif
#endif
