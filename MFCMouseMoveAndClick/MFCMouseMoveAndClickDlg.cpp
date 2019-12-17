
// MFCMouseMoveAndClickDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MFCMouseMoveAndClick.h"
#include "MFCMouseMoveAndClickDlg.h"
#include "afxdialogex.h"
#include <random>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_ID						8
#define TIMER_ELAPSE_MIN				100
#define TIMER_ELAPSE_MAX				10000
#define MOUSE_POINT_RANDOM_RANGE		10

// Global Variables
CMFCMouseMoveAndClickDlg* _pDlg;
HHOOK _hMouseHook;
HHOOK _hKeyboardHook;
BOOL _bEnable;
POINT _pCurrnetMouse;
POINT _pStartMouse;
POINT _pNextMouse;
std::mt19937 _gen;
std::uniform_int_distribution<> _uidTimerElapse;
std::uniform_int_distribution<> _uidMousePoint;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCMouseMoveAndClickDlg dialog



CMFCMouseMoveAndClickDlg::CMFCMouseMoveAndClickDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCMOUSEMOVEANDCLICK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCMouseMoveAndClickDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CURRENT_MOUSE, CStatic_Current_Mouse);
	DDX_Control(pDX, IDC_STATIC_NEXT_MOUSE, CStatic_Next_Mouse);
	DDX_Control(pDX, IDC_STATIC_START_MOUSE, CStatic_Start_Mouse);
	DDX_Control(pDX, IDC_STATIC_START_MOUSE, CStatic_Start_Mouse);
	DDX_Control(pDX, IDC_STATIC_TIMER_ELAPSE, CStatic_Timer_Elapse);
}

BEGIN_MESSAGE_MAP(CMFCMouseMoveAndClickDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

void CALLBACK EXPORT TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime) {
	if (nTimerid == TIMER_ID) {
		KillTimer(_pDlg->m_hWnd, TIMER_ID);

		int iScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
		int iScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
		double fx = _pNextMouse.x * (65535.0f / (double)iScreenWidth);
		double fy = _pNextMouse.y * (65535.0f / (double)iScreenHeight);

		INPUT input = { 0 };
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
		input.mi.dx = (LONG)fx;
		input.mi.dy = (LONG)fy;
		SendInput(1, &input, sizeof(INPUT));

		RtlZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		::SendInput(1, &input, sizeof(INPUT));

		RtlZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		::SendInput(1, &input, sizeof(INPUT));
		
		_pNextMouse.x = _pStartMouse.x + _uidMousePoint(_gen);
		_pNextMouse.y = _pStartMouse.y + _uidMousePoint(_gen);
		CString csTemp;
		csTemp.Format(L"Next Mouse: %d, %d", _pNextMouse.x, _pNextMouse.y);
		::SetWindowText(::GetDlgItem(_pDlg->m_hWnd, IDC_STATIC_NEXT_MOUSE), csTemp.GetString());

		UINT uTimeElapse = _uidTimerElapse(_gen);
		csTemp.Format(L"Time Elapse: %d ms", uTimeElapse);
		::SetWindowText(::GetDlgItem(_pDlg->m_hWnd, IDC_STATIC_TIMER_ELAPSE), csTemp.GetString());

		SetTimer(_pDlg->m_hWnd, TIMER_ID, uTimeElapse, TimerProc);
	}
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (wParam == WM_MOUSEMOVE || wParam == WM_LBUTTONDOWN) {
		GetCursorPos(&_pCurrnetMouse);
		CString csTemp;
		csTemp.Format(L"Current Mouse: %d, %d", _pCurrnetMouse.x, _pCurrnetMouse.y);
		::SetWindowText(::GetDlgItem(_pDlg->m_hWnd, IDC_STATIC_CURRENT_MOUSE), csTemp.GetString());
	}
	return CallNextHookEx(_hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (wParam == WM_KEYDOWN) {
		KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
		if (pKey->vkCode == VK_F5) {
			_bEnable ^= 1;
			CString csTemp;
			csTemp.Format((_bEnable) ? L"Enable" : L"Disable");
			_pDlg->SetWindowText(csTemp.GetString());

			if (_bEnable) {
				_pStartMouse = _pCurrnetMouse;
				_pNextMouse = _pCurrnetMouse;
				CString csTemp;
				csTemp.Format(L"Start Mouse: %d, %d", _pStartMouse.x, _pStartMouse.y);
				::SetWindowText(::GetDlgItem(_pDlg->m_hWnd, IDC_STATIC_START_MOUSE), csTemp.GetString());
				SetTimer(_pDlg->m_hWnd, TIMER_ID, 0, TimerProc);
			}
			else {
				KillTimer(_pDlg->m_hWnd, TIMER_ID);
			}
		}
	}
	return CallNextHookEx(_hKeyboardHook, nCode, wParam, lParam);
}

// CMFCMouseMoveAndClickDlg message handlers

BOOL CMFCMouseMoveAndClickDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	_pDlg = this;
	HMODULE hInstance = GetModuleHandle(NULL);
	_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, hInstance, 0);
	_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);

	// Random integer generator
	std::random_device rd;
	_gen = std::mt19937(rd());
	_uidTimerElapse = std::uniform_int_distribution<>(TIMER_ELAPSE_MIN, TIMER_ELAPSE_MAX);
	_uidMousePoint = std::uniform_int_distribution<>(0, MOUSE_POINT_RANDOM_RANGE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCMouseMoveAndClickDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_CLOSE) {
		UnhookWindowsHookEx(_hMouseHook);
		UnhookWindowsHookEx(_hKeyboardHook);
	}
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCMouseMoveAndClickDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCMouseMoveAndClickDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

