#pragma once
#include <vector>

namespace e
{
	typedef struct Region {
		int x0, y0, x1, y1, w, h;
		std::vector<cv::Point> points;
		Region(void) { clear(); }
		void clear(void) {
			x0 = y0 = x1 = y1 = w = h = 0;
			points.clear();
		}
	}Region;

	class CRegionFinder
	{
	public:
		CRegionFinder(int nWidth, int nHeight);
		virtual ~CRegionFinder(void);
	public:
		void FindRegion(std::vector<Region>& regions, void* pData, int nSize, int nWidth, int nHeight, int nBitCount, 
			bool bBlackPixel, int nMinSize, int nMaxSize);
		void FindRegion(std::vector<Region>& regions, cv::Rect roi, void* pData, int nSize, int nWidth, int nHeight, int nBitCount, 
			bool bBlackPixel, int nMinSize, int nMaxSize);
	protected:
		cv::Rect m_roiRect;
		cv::Point* m_pQueue;
		CBitmap* m_pTemp;
	};
}
