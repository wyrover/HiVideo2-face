#pragma once
namespace e
{
#ifndef WidthBytes
#define WidthBytes(bits) ((((bits)+31) & (~31)) / 8)
#endif

#ifndef _RGB
#define _RGB(r,g,b) ((uint)(((byte)(r)|((ushort)((byte)(g))<<8))|(((uint)(byte)(b))<<16)))
#endif

	void DrawLine(int x0, int y0, int x1, int y1, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	void DrawRect(int x, int y, int w, int h, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
	void DrawPoint(int x, int y, int nPenSize, int nColor, void* pData, int nSize, int nWidth, int nHeight, int nBitCount);
}

