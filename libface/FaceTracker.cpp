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

	template<class T> void limit(T & t, T a, T b)
	{
		if (t < a)
			t = a;
		else if (t > b)
			t = b;
	}

	CFaceTracker::CFaceTracker(void)
		: m_pMat(NULL)
		, m_bFaceLocated(false)
		, m_faceSize(0, 0)
		, m_minFaceSize(10, 10)
		, m_maxFaceSize(200, 200)
		, m_faceSizeOffset(5, 5)
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
		if (m_pMat)
		{
			delete m_pMat;
			m_pMat = 0;
		}
		if (m_pFaceCascade)
		{
			delete m_pFaceCascade;
			m_pFaceCascade = 0;
		}
	}

	void CFaceTracker::SetupCvMat(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (m_pMat == nullptr)
		{
			m_pMat = new cv::Mat(nHeight, nHeight, CV_8UC4, pData);
		}
		else
		{
			memcpy(m_pMat->ptr(), pData, nSize);
		}
	}

	void CFaceTracker::DrawRect(int x, int y, int w, int h, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		int x0 = x, y0 = y, x1 = x0 + w, y1 = y0 + h;
		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);
		limit(x0, 0, nWidth - 1);
		limit(y0, 0, nHeight - 1);
		limit(x1, 0, nWidth - 1);
		limit(y1, 0, nHeight - 1);

		byte r = 0xff, g = 0x00, b = 0x00;
		const int nPenSize = 1;
		const int nLineSize = WidthBytes(nBitCount*nWidth);
		const int nPixelSize = nBitCount / 8;
		//Õ˘ƒ⁄≤ø ’Àı
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

	void CFaceTracker::SetMinFaceSize(int nWidth, int nHeight)
	{
		m_minFaceSize.width = nWidth;
		m_minFaceSize.height = nHeight;
	}

	void CFaceTracker::SetMaxFaceSize(int nWidth, int nHeight)
	{
		m_maxFaceSize.width = nWidth;
		m_maxFaceSize.height = nHeight;
	}

	void CFaceTracker::Reset(void)
	{
		m_faceSize = cv::Size(0, 0);
		m_minFaceSize = cv::Size(10, 10);
		m_maxFaceSize = cv::Size(200, 200);
	}

	void CFaceTracker::OnSampleProc(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		const float fScale = 0.25f;
		const float fInvSacle = 1 / fScale;

		if (!m_bFaceLocated)
		{
			static int nTrackTimes = 0;
			TCHAR szText[256] = { 0 };
			_stprintf_s(szText, _T("%d: face track retry!\n"), nTrackTimes++);
			OutputDebugString(szText);

			Reset();
			cv::Mat mat(nHeight, nWidth, CV_8UC4, pData);
			cv::resize(mat, mat, cv::Size(), fScale, fScale, CV_INTER_LINEAR);
			std::vector<cv::Rect> faces;
			m_pFaceCascade->detectMultiScale(mat, faces, 1.1, 2, kHaarOptions, m_minFaceSize, m_maxFaceSize);

			m_bFaceLocated = false;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect & r = faces[i];
				m_faceSize.width = r.width;
				m_faceSize.height = r.height;

				r.x *= fInvSacle;
				r.y *= fInvSacle;
				r.width *= fInvSacle;
				r.height *= fInvSacle;
				DrawRect(r.x, r.y, r.width, r.height, pData, nSize, nWidth, nHeight, nBitCount);
				m_bFaceLocated = true;
			}
		}
		else
		{
			m_minFaceSize = m_faceSize - m_faceSizeOffset;
			m_maxFaceSize = m_faceSize + m_faceSizeOffset;
			cv::Mat mat(nHeight, nWidth, CV_8UC4, pData);
			cv::resize(mat, mat, cv::Size(), fScale, fScale, CV_INTER_LINEAR);

			std::vector<cv::Rect> faces;
			m_pFaceCascade->detectMultiScale(mat, faces, 1.1, 2, kHaarOptions, m_minFaceSize, m_maxFaceSize);

			m_bFaceLocated = false;
			for (size_t i = 0; i < faces.size(); i++)
			{
				cv::Rect & r = faces[i];
				m_faceSize.width = r.width;
				m_faceSize.height = r.height;

				r.x *= fInvSacle;
				r.y *= fInvSacle;
				r.width *= fInvSacle;
				r.height *= fInvSacle;
				DrawRect(r.x, r.y, r.width, r.height, pData, nSize, nWidth, nHeight, nBitCount);
				m_bFaceLocated = true;
			}
		}
	}
}