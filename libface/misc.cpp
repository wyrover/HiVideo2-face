#include "stdafx.h"
#include "misc.h"

namespace e
{
	typedef unsigned char byte;

	template<class T> void limit(T & t, T a, T b)
	{
		if (t < a)
			t = a;
		else if (t > b) 
			t = b;
	}

	void DrawLine(int x0, int y0, int x1, int y1, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);
		limit(x0, 0, nWidth - 1);
		limit(y0, 0, nHeight - 1);
		limit(x1, 0, nWidth - 1);
		limit(y1, 0, nHeight - 1);

		const byte b = nColor & 0x000000ff;
		const byte g = (nColor & 0x0000ff00) >> 8;
		const byte r = (nColor & 0x00ff0000) >> 16;
		const int nLineSize = WidthBytes(nBitCount*nWidth);
		const int nPixelSize = nBitCount / 8;

		if (y0 == y1)
		{
			for (int i = 0; i < nPenSize; i++)
			{
				byte* p = (byte*)pData + min(y0 + i, nHeight-1) * nLineSize + x0 * nPixelSize;
				for (int x = x0; x <= x1; x++)
				{
					*(p + 0) = b;
					*(p + 1) = g;
					*(p + 2) = r;
					p += nPixelSize;
				}
			}
		}
		else if (x0 == x1)
		{
			for (int i = 0; i < nPenSize; i++)
			{
				byte* p = (byte*)pData + y0 * nLineSize + min(x0 + i, nWidth-1) * nPixelSize;
				for (int y = y0; y <= y1; y++)
				{
					*(p + 0) = b;
					*(p + 1) = g;
					*(p + 2) = r;
					p += nLineSize;
				}
			}
		}
	}

	void DrawRect(int x, int y, int w, int h, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
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

			p0 = (byte*)pData + y0 * nLineSize + min(x0 + i, x1) * nPixelSize;
			p1 = (byte*)pData + y0 * nLineSize + max(x1 - i, x0) * nPixelSize;

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

	void DrawPoint(int x, int y, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		assert(x >= 0 && x < nWidth);
		assert(y >= 0 && y < nHeight);
		limit(x, 0, nWidth - 1);
		limit(y, 0, nHeight - 1);
		const byte b = nColor & 0x000000ff;
		const byte g = (nColor & 0x0000ff00) >> 8;
		const byte r = (nColor & 0x00ff0000) >> 16;
		const int nPixelSize = nBitCount / 8;
		const int nLineSize = WidthBytes(nWidth*nBitCount);

		byte* p = (byte*)pData + y * nLineSize + x * nPixelSize;
		*(p + 0) = b; *(p + 1) = g; *(p + 2) = r;
	}
}
