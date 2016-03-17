// TestCapture.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<stdio.h>
#include <opencv2\opencv.hpp>
using namespace std;

int main(int argc, char** argv)
{
	//声明IplImage指针
	IplImage* pFrame = NULL;
	//获取摄像头
	CvCapture* pCapture = cvCaptureFromCAM(0);// cvCreateCameraCapture(0);
	cvWaitKey(200);
	//创建窗口
	cvNamedWindow("Video", 1);

	//显示视屏
	while (1)
	{
		pFrame = cvQueryFrame(pCapture);
		if (!pFrame)break;
		cvShowImage("Video", pFrame);
		char c = cvWaitKey(10);
		if (c == 27)break;

		static int count = 0;
		printf("frame count: %d\n", count++);
	}
	cvReleaseCapture(&pCapture);
	cvDestroyWindow("Video");
	return 0;
}
