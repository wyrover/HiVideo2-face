#pragma once

namespace e
{
	class CFaceTracker
	{
	public:
		CFaceTracker(void);
		virtual ~CFaceTracker(void);
		void SetMinFaceSize(int nWidth, int nHeight);
		void SetMaxFaceSize(int nWidth, int nHeight);
		void OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	protected:
		void SetupCvMat(void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
		void DrawRect(int x
			, int y
			, int w
			, int h
			, void* pData
			, int nSize
			, int nWidth
			, int nHeight
			, int nBitCount);
		void Reset(void);
	protected:
		cv::Mat* m_pMat;
		cv::CascadeClassifier* m_pFaceCascade;
		cv::Size m_faceSize;
		cv::Size m_minFaceSize;
		cv::Size m_maxFaceSize;
		cv::Size m_faceSizeOffset;
		bool m_bFaceLocated;
	};
}