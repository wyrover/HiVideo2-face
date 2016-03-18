#include "stdafx.h"
#include "FaceTracker.h"
#include "misc.h"

namespace e
{
	const int kMaxRetryTimes = 2;
	const int kHaarFaceOptions = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
	const int kHaarEyesOptions = CV_HAAR_SCALE_IMAGE;
	//const char* kFaceCascade = "D:\\ThirdParty\\opencv-3.1\\etc\\lbpcascades\\lbpcascade_frontalface.xml";
	const char* kFaceCascade = "D:\\ThirdParty\\opencv-3.1\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml";
	const char* kEyesCascade = "D:\\ThirdParty\\opencv-3.1\\etc\\haarcascades\\haarcascade_eye_tree_eyeglasses.xml";

	const float kDefaultScale = 0.25f;
	const cv::Size kDefaultMinFaceSize(10, 10);
	const cv::Size kDefaultMaxFaceSize(60, 60);

	template<class T> void limit(T & t, T a, T b)
	{
		if (t < a)
			t = a;
		else if (t > b)
			t = b;
	}

	void limit(cv::Rect & rect, int w, int h)
	{
		limit(rect.x, 0, w - 1);
		limit(rect.y, 0, h - 1);
		limit(rect.width, 0, w - rect.x);
		limit(rect.height, 0, h - rect.y);
	}

	CFaceTracker::CFaceTracker(void)
		: m_pSrcMat(NULL)
		, m_bFaceLocated(false)
		, m_dwLastTime(0)
	{
		//m_pMat = new cv::Mat();
		m_pFaceCascade = new cv::CascadeClassifier();
		if (!m_pFaceCascade->load(kFaceCascade))
		{
			LOGE(_T("face cascade load data failed!"));
			assert(0);
		}

		const char* pFileName = "f:\\flandmark_model.dat";
		m_pFlandmarkModel = flandmark_init(pFileName);
		if (!m_pFlandmarkModel)
		{
			LOGE(_T("flandmark model init failed!"));
			assert(0);
		}
	}

	CFaceTracker::~CFaceTracker(void)
	{
		if (m_pSrcMat)
		{
			m_pSrcMat->data.ptr = 0;
			cvReleaseMat(&m_pSrcMat);
			m_pSrcMat = 0;
		}

		if (m_pFaceCascade)
		{
			delete m_pFaceCascade;
			m_pFaceCascade = 0;
		}

		if (m_pFlandmarkModel)
		{
			flandmark_free(m_pFlandmarkModel);
			m_pFlandmarkModel = 0;
		}
	}

	void CFaceTracker::SetupCvMat(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (m_pSrcMat == nullptr)
		{
			m_pSrcMat = cvCreateMatHeader(nHeight, nWidth, CV_8UC4);
			m_dstMat.create(cv::Size(nWidth*kDefaultScale, nHeight*kDefaultScale), CV_8UC4);
		}

		m_pSrcMat->data.ptr = (byte*)pData;
		m_pSrcMat->step = WidthBytes(nBitCount*nWidth);

		cv::Mat srcMat = cvarrToMat(m_pSrcMat, false);
		cv::resize(srcMat, m_dstMat, cv::Size(), kDefaultScale, kDefaultScale, CV_INTER_LINEAR);
	}

	void CFaceTracker::SetupCvMat(cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (m_pSrcMat == nullptr)
		{
			m_pSrcMat = cvCreateMatHeader(nHeight, nWidth, CV_8UC4);
			m_dstMat.create(cv::Size(nWidth*kDefaultScale, nHeight*kDefaultScale), CV_8UC4);
		}

		m_pSrcMat->data.ptr = (byte*)pData;
		m_pSrcMat->step = WidthBytes(nBitCount*nWidth);

		cv::Mat srcMat = cvarrToMat(m_pSrcMat, false);
		cv::Mat roiMat = cv::Mat(srcMat, roi);
		cv::resize(roiMat, m_dstMat, cv::Size(), kDefaultScale, kDefaultScale, CV_INTER_LINEAR);
	}

	void CFaceTracker::DrawRect(cv::Rect rect, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		e::DrawRect(rect.x, rect.y, rect.width, rect.height, nPenSize, nColor, pData, nSize, nWidth, nHeight, nBitCount);
	}

	void CFaceTracker::DrawPoint(std::vector<cv::Point>& points, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		for (size_t i = 0; i < points.size(); i++)
		{
			cv::Point & p = points[i];
			e::DrawPoint(p.x, p.y, nPenSize, nColor, pData, nSize, nWidth, nHeight, nBitCount);
		}
	}

	void CFaceTracker::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//首次人脸检测
		if (!m_bFaceLocated)
		{
			DWORD dwTime = GetTickCount();
			if (m_dwLastTime != 0 && dwTime - m_dwLastTime < 500) return;

			SetupCvMat(pData, nSize, nWidth, nHeight, nBitCount);
			std::vector<cv::Rect> faces;
			m_pFaceCascade->detectMultiScale(m_dstMat, faces, 1.1, 2, kHaarFaceOptions, kDefaultMinFaceSize, kDefaultMaxFaceSize);

			const float fInvScale = 1.0f / kDefaultScale;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect rect = faces[i];
				rect.x *= fInvScale;
				rect.y *= fInvScale;
				rect.width *= fInvScale;
				rect.height *= fInvScale;
				m_faceRect = rect;
				m_prevRect = rect;
				DrawRect(rect, 2, _RGB(255, 0, 0), pData, nSize, nWidth, nHeight, nBitCount);
				m_bFaceLocated = true;
			}

			m_dwLastTime = dwTime;
			static int nTrackTimes = 0;
			LOGD(_T("face tracek restart: %d"), nTrackTimes++);
		}
		else//根据前一帧检测的数据，进行跟踪处理
		{
			int nRetryTimes = 0;
			const float fScale = kDefaultScale;
			const float fInvScale = 1.0f / fScale;
			const float fdx = nWidth * 0.05f;
			const float fdy = nHeight * 0.05f;
			cv::Rect expendRect = m_faceRect;
_again:
			expendRect.x -= fdx;
			expendRect.y -= fdy;
			expendRect.width += 2*fdx;
			expendRect.height += 2*fdy;
			limit(expendRect, nWidth, nHeight);
			//display search erea
			DrawRect(expendRect, 2, _RGB(0,0,255), pData, nSize, nWidth, nHeight, nBitCount);

			SetupCvMat(expendRect, pData, nSize, nWidth, nHeight, nBitCount);
			//setup face size
			std::vector<cv::Rect> faces;
			cv::Size faceSize(m_faceRect.width*fScale, m_faceRect.height*fScale);
			cv::Size sizeOffset(faceSize.width*0.25f, faceSize.height*0.25f);
			cv::Size minFaceSize = faceSize - sizeOffset;
			cv::Size maxFaceSize = faceSize + sizeOffset;
			m_pFaceCascade->detectMultiScale(m_dstMat, faces, 1.1, 2, kHaarFaceOptions, minFaceSize, maxFaceSize);

			m_bFaceLocated = false;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect rect = faces[i];
				rect.x *= fInvScale;
				rect.y *= fInvScale;
				rect.width *= fInvScale;
				rect.height *= fInvScale;
				rect.x += expendRect.x;
				rect.y += expendRect.y;
				limit(rect, nWidth, nHeight);
				
				if (m_faceRect.contains(rect.tl()) && m_faceRect.contains(rect.br()))
				{
					rect = m_faceRect;
				}
				else if (rect.contains(m_faceRect.tl()) && rect.contains(m_faceRect.br()))
				{
					m_faceRect = rect;
				}

				int dx = abs(rect.x - m_faceRect.x);
				int dy = abs(rect.y - m_faceRect.y);
				if (dx >= 5 || dy >= 5)
				{
					m_faceRect = rect;
				}
				m_prevRect = rect;
				m_bFaceLocated = true;

				DrawRect(m_faceRect, 2, _RGB(0,255,0), pData, nSize, nWidth, nHeight, nBitCount);

//眼睛、鼻子、嘴巴定位处理
				cv::Mat gray;
				cv::Mat faceMat = cv::Mat(cvarrToMat(m_pSrcMat), expendRect);
				cv::cvtColor(faceMat, gray, CV_RGB2GRAY);
				cv::resize(gray, gray, cv::Size(), fScale, fScale, CV_INTER_LINEAR);
				cv::imwrite("f:\\temp\\gray.bmp", gray);

				rect = m_prevRect;
				rect.x -= expendRect.x;
				rect.y -= expendRect.y;
				rect.x *= fScale;
				rect.y *= fScale;
				rect.width *= fScale;
				rect.height *= fScale;

				int bbox[4] = { rect.x, rect.y, rect.x+rect.width, rect.y+rect.height };
				LOGD(_T("flandmark: %d,%d,%d,%d"), bbox[0], bbox[1], bbox[2], bbox[3]);

				double *landmarks = (double*)malloc(2 * m_pFlandmarkModel->data.options.M * sizeof(double));
				IplImage* input = &IplImage(gray);
				flandmark_detect(input, bbox, m_pFlandmarkModel, landmarks);
	
				cvCircle(m_pSrcMat
					, cvPoint(int(landmarks[0]* fInvScale + expendRect.x), int(landmarks[1]* fInvScale + expendRect.y))
					, 3, CV_RGB(0, 0, 255)
					, CV_FILLED);

				for (int i = 2; i < 2 * m_pFlandmarkModel->data.options.M; i += 2)
				{
					cvCircle(m_pSrcMat
						, cvPoint(int(landmarks[i]* fInvScale + expendRect.x), int(landmarks[i + 1]* fInvScale + expendRect.y))
						, 3, CV_RGB(255, 0, 0)
						, CV_FILLED);
				}
				free(landmarks);		
			}// end for each face

			//跟踪失败，再次检测或结束
			if (!m_bFaceLocated)
			{
				static int nFailedTimes = 0;
				LOGD(_T("face track failed: %d"), nFailedTimes++);
				cv::imwrite("f:\\temp\\loss.bmp", m_dstMat);
				if (++nRetryTimes < kMaxRetryTimes) goto _again;           
			}
		}
	}
}