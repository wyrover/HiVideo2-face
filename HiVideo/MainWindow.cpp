#include "stdafx.h"
#include "MainWindow.h"
#include "UIVideo.h"
#include "UICurve.h"
#include "VideoDevice.h"

namespace e
{

	class CHiCameraBuilderCallback : public IDialogBuilderCallback
	{
	public:
		virtual CControlUI* CreateControl(LPCTSTR pstrName)
		{
			if (_tcscmp(pstrName, _T("Curve")) == 0)
			{
				return new CCurveUI();
			}
			else if (_tcscmp(pstrName, _T("Video")) == 0)
			{
				return new CVideoUI();
			}
			return NULL;
		}
	};
	CMainWindow::CMainWindow(void)
		: m_pVideoDevice(NULL)
		, m_bFaceTrack(FALSE)
	{
		HRESULT hr;
		m_pVideoDevice = new CVideoDevice(&hr);
		ASSERT(m_pVideoDevice);

		m_pFaceTracker = new CFaceTracker();
		ASSERT(m_pFaceTracker);
	}

	CMainWindow::~CMainWindow(void)
	{
		if (m_pVideoDevice) delete m_pVideoDevice;
		if (m_pFaceTracker) delete m_pFaceTracker;
	}

	void CMainWindow::Init(void)
	{
		InitDevice();
	}

	LRESULT CMainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::PostQuitMessage(0L);
		bHandled = FALSE;
		return 0;
	}

	LRESULT CMainWindow::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (::IsIconic(*this)) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT CMainWindow::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT CMainWindow::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT CMainWindow::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
				return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT CMainWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT  CMainWindow::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT CMainWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT CMainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		CDialogBuilder builder;
		CHiCameraBuilderCallback callback;
		CControlUI* pRoot = builder.Create(_T("MainFrame.xml"), (UINT)0, &callback, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		Init();
		return 0;
	}

	LRESULT CMainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg) {
		case WM_CREATE:				lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:			lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:     lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:			lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:					lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:				lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:			lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		default: bHandled = FALSE;break;
		}

		if (bHandled) return lRes;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;

		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	void CMainWindow::OnMinimize(void)
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}

	void CMainWindow::OnMaximize(void)
	{
		if (::IsZoomed(m_pm.GetPaintWindow()))
		{
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		}
		else
		{
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
	}

	void CMainWindow::Notify(TNotifyUI& msg)
	{
		TCHAR szMsg[256];
		_stprintf_s(szMsg, _T("msg type = %s\n"), msg.sType.GetData());
		OutputDebugString(szMsg);

		if (msg.sType == _T("click"))
		{
			if (msg.pSender->GetName() == _T("btn_close") || msg.pSender->GetName() == _T("closebtn2")){
				Close();
			}else if (msg.pSender->GetName() == _T("btn_maxbox") || msg.pSender->GetName() == _T("maxboxbtn2")){
				OnMaximize();
			}else if (msg.pSender->GetName() == _T("btn_minbox") || msg.pSender->GetName() == _T("minboxbtn2")){
				OnMinimize();
			}else if (msg.pSender->GetName() == _T("btn_video_start")){
				OnVideoStart();
			}else if (msg.pSender->GetName() == _T("btn_video_stop")){
				OnVideoStop();
			}else if (msg.pSender->GetName() == _T("btn_video_detect")){
				OnVideoDetect();
			}
		}
		else if (msg.sType == _T("itemselect"))
		{
			if (msg.pSender->GetName() == _T("com_device_list")){
				SwitchDevice();
			}else if (msg.pSender->GetName() == _T("com_format_list")){
				SwitchFormat();
			}
		}
		else if (msg.sType == _T("valuechanged"))
		{

		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//----------------------------------------Camera--------------------------------------------------
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool CMainWindow::InitDevice(void)
	{
		HRESULT hr;
		hr = m_pVideoDevice->GetCaptureDeviceNames(this);
		if (FAILED(hr)) return false;

		auto pDeviceName = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_device_list")))->GetText();
		hr = m_pVideoDevice->GetCaptureDeviceFormats(pDeviceName, this);
		return SUCCEEDED(hr);
	}

	bool CMainWindow::SwitchDevice(void)
	{
		//devices
		{
			auto pControl = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_format_list")));
			pControl->RemoveAll();
		}
		//formats
		{
			auto pControl = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_device_list")));
			HRESULT hr = m_pVideoDevice->GetCaptureDeviceFormats(pControl->GetText(), this);
			return SUCCEEDED(hr);
		}
	}

	bool CMainWindow::SwitchFormat(void)
	{
		auto pControl = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_format_list")));
		HRESULT hr = m_pVideoDevice->SetCurrentOutputFormat(pControl->GetText());
		return SUCCEEDED(hr);
	}

	void CMainWindow::OnAddDevice(const TCHAR* pszDeviceName)
	{
		CListLabelElementUI* pLabel = new CListLabelElementUI();
		pLabel->SetText(pszDeviceName);
		auto pControl = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_device_list")));
		pControl->Add(pLabel);
		pControl->SelectItem(0);
	}

	void CMainWindow::OnAddFormat(const TCHAR* pszFormat)
	{
		CListLabelElementUI* pLabel = new CListLabelElementUI();
		pLabel->SetText(pszFormat);
		auto pControl = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_format_list")));
		pControl->Add(pLabel);
		pControl->SelectItem(0);
	}

	void CMainWindow::OnAddSample(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		//render ref video
		static auto pRefVideo = static_cast<DuiLib::CVideoUI*>(m_pm.FindControl(_T("src_video")));
		static auto pMixVideo = static_cast<DuiLib::CVideoUI*>(m_pm.FindControl(_T("dst_video")));

		pRefVideo->DoRenderSample(pData, nWidth, nHeight, nBitCount);

		if (m_bFaceTrack)
		{
			OnFaceTrack(pData, nSize, nWidth, nHeight, nBitCount);			
			pMixVideo->DoRenderSample(pData, nWidth, nHeight, nBitCount);
		}
		else
		{
			DWORD dwGreenColor = 0xff00ff00;
			pMixVideo->DoRenderSample(dwGreenColor);
		}
	}

	void CMainWindow::OnVideoStart(void)
	{
		HRESULT  hr;
		auto pDeviceName = static_cast<DuiLib::CComboUI*>(m_pm.FindControl(_T("com_device_list")))->GetText();
		hr = m_pVideoDevice->Create(pDeviceName, this);
		if (FAILED(hr)) return;
		hr = m_pVideoDevice->Start();
		if (FAILED(hr)) return;
		m_dwStartTime = 0;
	}

	void CMainWindow::OnVideoStop(void)
	{
		HRESULT hr = m_pVideoDevice->Stop();
		if (FAILED(hr)) return;

		DWORD dwGreenColor = 0xff00ff00;
		auto pControl = static_cast<DuiLib::CVideoUI*>(m_pm.FindControl(_T("src_video")));
		pControl->DoRenderSample(dwGreenColor);
		pControl = static_cast<DuiLib::CVideoUI*>(m_pm.FindControl(_T("dst_video")));
		pControl->DoRenderSample(dwGreenColor);
	}

	void CMainWindow::OnVideoDetect(void)
	{
		m_bFaceTrack = !m_bFaceTrack;
	}

	void CMainWindow::OnFaceTrack(void* pData, int nSize, int nWidth, int nHeight, int nBitCount)
	{
		if (m_pFaceTracker)
		{
			m_pFaceTracker->OnSampleProc(pData, nSize, nWidth, nHeight, nBitCount);
		}
	}

}
