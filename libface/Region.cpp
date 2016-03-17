#include "stdafx.h"
#include "Region.h"

namespace e
{
	CRegionFinder::CRegionFinder(int nWidth, int nHeight)
	{
		m_pQueue = new cv::Point[nWidth * nHeight];
		assert(m_pQueue);
		m_pTemp = new CBitmap(nWidth, nHeight, 8);
		assert(m_pTemp);
	}

	CRegionFinder::~CRegionFinder(void)
	{
		SafeDelete(&m_pQueue);
		SafeDelete(&m_pTemp);
	}

	void CRegionFinder::FindRegion(std::vector<Region>& regions, void* pData, int nSize, int nWidth, int nHeight, int nBitCount, 
		bool bBlackPixel, int nMinSize, int nMaxSize)
	{
		cv::Rect roi(1, 1, nWidth-2, nHeight-2);
		FindRegion(regions, roi, pData, nSize, nWidth, nHeight, nBitCount, bBlackPixel, nMinSize, nMaxSize);
	}

	void CRegionFinder::FindRegion(std::vector<Region>& regions, cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, 
		int nBitCount, bool bBlackPixel, int nMinSize, int nMaxSize)
	{
		assert(nBitCount == 8);
		CBitmap bitmap(nWidth, nHeight, nBitCount, pData, false);
		m_pTemp->Store(&bitmap);

		int x0, y0, x1, y1;
		int nHead, nRear, nCurrentRegion;
		int nKeepValue = 0, nWipeValue = 0;
		if (bBlackPixel) {
			nWipeValue = 0; nKeepValue = 255;
		}else {
			nWipeValue = 255; nKeepValue = 0;
		}

		Region region;
		nHead = nRear = 0;
		nCurrentRegion = 0;

		static int nDirection[4][2] = { { 1, 0 },{ -1, 0 },{ 0, 1 },{ 0, -1 } };
		for (int y = roi.y; y < roi.height; y++)
		{
			for (int x = roi.x; x < roi.width; x++)
			{
				BYTE* pCurrent = m_pTemp->GetBits(x, y);
				if (*pCurrent != nWipeValue) continue;
				*pCurrent = nKeepValue;

				cv::Point point(x, y);
				m_pQueue[nRear++] = point;
				region.points.push_back(point);

				while (nHead < nRear)
				{
					point = m_pQueue[nHead++];
					x0 = point.x;
					y0 = point.y;
					for (int i = 0; i < 4; i++)
					{
						x1 = x0 + nDirection[i][0];
						y1 = y0 + nDirection[i][1];
						if ((x1 >= 0 && x1 < roi.width) && (y1 >= 0 && y1 < roi.height))
						{
							BYTE* pNext = m_pTemp->GetBits(x1, y1);
							if (*pNext == nWipeValue)
							{
								*pNext = nKeepValue;
								point.x = x1;point.y = y1;
								m_pQueue[nRear++] = point;
								if (x1 < region.x0) region.x0 = x1;
								else if (x1 > region.y1) region.y1 = y1;
								if (y1 < region.y0) region.y0 = y1;
								else if (y1 > region.y1) region.y1 = y1;
								region.points.push_back(point);
							}
						}
					}
				}//end while (nHead < nRear)

				//结束一个区域的查找
				size_t nSize = region.points.size();
				if (nSize >= (size_t)nMinSize && nSize <= (size_t)nMaxSize)
				{
					region.w = region.x1 - region.x0 + 1;
					region.h = region.y1 - region.y0 + 1;
					regions.push_back(region);
				}
				//clear for netxt
				region.clear();
			}
		}
	}
}
