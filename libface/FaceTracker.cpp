#include "stdafx.h"
#include "FaceTracker.h"
#ifndef WidthBytes
#define WidthBytes(bits) ((((bits)+31) & (~31)) / 8)
#endif
namespace e
{
	typedef unsigned char byte;
	const int kHaarOptions = CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH;
	const char* kFaceCascade = "D:\\ThirdParty\\opencv-3.1\\etc\\haarcascades\\haarcascade_frontalface_alt2.xml";

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
			OutputDebugString(_T("face track load data failed!\n"));
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

	void CFaceTracker::DrawRect(int x
		, int y
		, int w
		, int h
		, int nPenSize
		, int nColor
		, void* pData
		, int nSize
		, int nWidth
		, int nHeight
		, int nBitCount)
	{
		int x0 = x, y0 = y, x1 = x0 + w, y1 = y0 + h;
		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);
		limit(x0, 0, nWidth - 1);
		limit(y0, 0, nHeight - 1);
		limit(x1, 0, nWidth - 1);
		limit(y1, 0, nHeight - 1);

		byte b = nColor & 0x000000ff;
		byte g = (nColor & 0x0000ff00) >> 8;
		byte r = (nColor & 0x00ff0000) >> 16;
		const int nLineSize = WidthBytes(nBitCount*nWidth);
		const int nPixelSize = nBitCount / 8;
		//ÍùÄÚ²¿ÊÕËõ
		for (int i = 0; i < nPenSize; i++)
		{
			byte* p0 = (byte*)pData + min(y0 + i, y1) * nLineSize + x0 * nPixelSize;
			byte* p1 = (byte*)pData + max(y1 - i, y0)* nLineSize + x0 * nPixelSize;

			for (int x = x0; x <= x1; x++)
			{
				*(p0 + 0) = b;
				*(p0 + 1) = g;
				*(p0 + 2) = r;

				*(p1 + 0) = b;
				*(p1 + 1) = g;
				*(p1 + 2) = r;

				p0 += nPixelSize;
				p1 += nPixelSize;
			}

			p0 = (byte*)pData + y0 * nLineSize + min(x0+i, x1) * nPixelSize;
			p1 = (byte*)pData + y0 * nLineSize + max(x1-i, x0) * nPixelSize;

			for (int y = y0; y <= y1; y++)
			{
				*(p0 + 0) = b;
				*(p0 + 1) = g;
				*(p0 + 2) = r;

				*(p1 + 0) = b;
				*(p1 + 1) = g;
				*(p1 + 2) = r;

				p0 += nLineSize;
				p1 += nLineSize;
			}
		}
	}

	void CFaceTracker::DrawRect(cv::Rect rect, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		DrawRect(rect.x, rect.y, rect.width, rect.height, nPenSize, nColor, pData, nSize, nWidth, nHeight, nBitCount);
	}

	void CFaceTracker::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (!m_bFaceLocated)
		{
			DWORD dwTime = GetTickCount();
			if (m_dwLastTime != 0 && dwTime - m_dwLastTime < 500) return;

			static int nTrackTimes = 0;
			TCHAR szText[256] = { 0 };
			_stprintf_s(szText, _T("%d: face track retry!\n"), nTrackTimes++);
			OutputDebugString(szText);

			SetupCvMat(pData, nSize, nWidth, nHeight, nBitCount);

			std::vector<cv::Rect> faces;
			m_pFaceCascade->detectMultiScale(m_dstMat, faces, 1.1, 2, kHaarOptions, kDefaultMinFaceSize, kDefaultMaxFaceSize);

			const float fInvScale = 1.0f / kDefaultScale;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect & r = faces[i];
				r.x *= fInvScale;
				r.y *= fInvScale;
				r.width *= fInvScale;
				r.height *= fInvScale;
				m_faceRect = r;
				m_prevRect = r;
				DrawRect(r.x, r.y, r.width, r.height, 2, 0xffff0000, pData, nSize, nWidth, nHeight, nBitCount);
				m_bFaceLocated = true;
			}

			m_dwLastTime = dwTime;
		}
		else
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

			DrawRect(expendRect, 2, 0xff0000ff, pData, nSize, nWidth, nHeight, nBitCount);

			SetupCvMat(expendRect, pData, nSize, nWidth, nHeight, nBitCount);
			//setup face size
			std::vector<cv::Rect> faces;
			cv::Size faceSize(m_faceRect.width*fScale, m_faceRect.height*fScale);
			cv::Size sizeOffset(faceSize.width*0.25f, faceSize.height*0.25f);
			cv::Size minFaceSize = faceSize - sizeOffset;
			cv::Size maxFaceSize = faceSize + sizeOffset;
			m_pFaceCascade->detectMultiScale(m_dstMat, faces, 1.1, 2, kHaarOptions, minFaceSize, maxFaceSize);

			m_bFaceLocated = false;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect & rect = faces[i];
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
				DrawRect(m_faceRect, 2, 0xff00ff00, pData, nSize, nWidth, nHeight, nBitCount);
			}
			//¸ú×ÙÊ§°ÜµÄÍ¼Ïñ
			if (!m_bFaceLocated)
			{
				OutputDebugString(_T("face track failed!\n"));
				cv::imwrite("f:\\temp\\loss.bmp", m_dstMat);
				if (++nRetryTimes < 2) goto _again;
			}
		}
	}
}