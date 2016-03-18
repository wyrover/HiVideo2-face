#pragma once
#include "flandmark.h"

namespace e
{
#define DRAW_FACERECT 1

	struct CTrackContext {
		cv::Rect prevRect;//absolute position
		cv::Rect faceRect;//absolute position
		bool faceLocated;
		uint32 lastTime;
	};

	class CRegionFinder;
	class CFaceTracker
	{
	public:
		CFaceTracker(void);
		virtual ~CFaceTracker(void);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		void SetupMatrix(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetupMatrix(cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void DrawRect(cv::Rect rect, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void DrawPoint(std::vector<cv::Point>& points, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		IplImage* m_pImage;	//only wrapper
		cv::Mat* m_pMatDetection;
		cv::Rect m_faceRect;
		cv::Rect m_prevRect;
		bool m_bFaceLocated;
		DWORD m_dwLastTime;
		cv::CascadeClassifier* m_pFaceCascade;
		//CTrackContext* m_pContext;
		CvMat* m_pAdapterMatrix;
		cv::Mat* m_pDetectMatrix;
		FLANDMARK_Model* m_pFlandmarkModel;
		double* m_plfFlandmark;
	};
}