#ifndef __E_IMAGE_H__
#define __E_IMAGE_H__

template<typename T>class CImage
{
public:
	CImage(int nWidth, int nHeight)
	{
		m_nSize = 0;
		m_nWidth = 0;
		m_nHeight = 0;
		m_pData = 0;
		m_pAccess = 0;
		Create(nWidth, nHeight);
	}
	virtual ~CImage(void)
	{
		Destroy();
	}
	int Width(void)
	{ return m_nWidth; }
	int Height(void)
	{ return m_nHeight; }
	int Size(void)
	{ return m_nSize; }
private:
	bool Create(int nWidth, int nHeight)
	{
		assert(nWidth > 0 && nHeight > 0);
		int nSize = nWidth * nHeight * sizeof(T);
		m_pData = (T*)realloc(m_pData, nSize);
		if (m_pData == NULL) return false;
		m_pAccess = (T**)realloc(m_pAccess, nHeight);
		if (m_pAccess == NULL) return false;
		for (int y = 0; y < nHeight; y++)
		{
			m_pAccess[i] = m_pData + y * nWidth;
		}
		m_nSize = nSize;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		return true;
	}
	void Destroy(void)
	{
		SafeFree(&m_pData);
	}
public:
	int m_nSize;
	int m_nWidth;
	int m_nHeight;
	T* m_pData;
	T** m_pAccess;
};



#endif