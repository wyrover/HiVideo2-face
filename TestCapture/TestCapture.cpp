// TestCapture.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<stdio.h>
#include <opencv2\opencv.hpp>
using namespace std;

int main(int argc, char** argv)
{
	//����IplImageָ��
	IplImage* pFrame = NULL;
	//��ȡ����ͷ
	CvCapture* pCapture = cvCaptureFromCAM(0);// cvCreateCameraCapture(0);
	cvWaitKey(200);
	//��������
	cvNamedWindow("Video", 1);

	//��ʾ����
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
