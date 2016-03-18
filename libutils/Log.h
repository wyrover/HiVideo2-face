#pragma once
namespace e
{
	class CLogger
	{
	public:
		CLogger(const TCHAR* pFileName);
		virtual ~CLogger(void);
	public:
		void d(const TCHAR* pFormat, ...);
		void i(const TCHAR* pFormat, ...);
		void w(const TCHAR* pFormat, ...);
		void e(const TCHAR* pFormat, ...);
	private:
		bool Open(const TCHAR* pFileName);
		bool Reserve(int nCapacity);
		void Write(const TCHAR* pPrefix, const TCHAR* pFormat, va_list arglist);
		void Write(const TCHAR* pszMsg, int nSize);
		void Close(void);
	private:
		HANDLE m_hFile;
		int m_nBufferSize;
		TCHAR* m_pBuffer;
		CRITICAL_SECTION m_csLock;
	};

	//only debug log with windows OS
	void Log(const TCHAR* pFormat, ...);
	void Log(const TCHAR* pPrefix, const TCHAR* pFormat, ...);

#define LOGI(...) Log(_T("info: "), __VA_ARGS__)
#define LOGE(...) Log(_T("error: "),  __VA_ARGS__)
#define LOGD(...) Log(_T("debug: "), __VA_ARGS__)
#define LOGW(...) Log(_T("warnning: "), __VA_ARGS__)
}

