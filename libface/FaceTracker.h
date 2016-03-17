#pragma once
#include "flandmark_detector.h"

namespace e
{
	class CRegionFinder;
	class CFaceTracker
	{
	public:
		CFaceTracker(void);
		virtual ~CFaceTracker(void);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		void SetupCvMat(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void SetupCvMat(cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void DrawRect(cv::Rect rect, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void DrawPoint(std::vector<cv::Point>& points, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		CvMat*  m_pSrcMat;
		cv::Mat m_dstMat;
		cv::CascadeClassifier* m_pFaceCascade;
		cv::Rect m_faceRect;
		cv::Rect m_prevRect;
		bool m_bFaceLocated;
		DWORD m_dwLastTime;

		FLANDMARK_Model* m_pFlandmarkModel;
	};
}