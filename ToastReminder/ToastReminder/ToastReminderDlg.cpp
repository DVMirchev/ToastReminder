
// ToastReminderDlg.cpp : implementation file
//

#include "stdafx.h"

#include <chrono>
#include <thread>
#include <future>
#include <ctime>
#include <ratio>

#include "ToastReminder.h"
#include "ToastReminderDlg.h"
#include "afxdialogex.h"
#include "ToastHelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std::chrono_literals;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CToastReminderDlg dialog



CToastReminderDlg::CToastReminderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TOASTREMINDER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CToastReminderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CToastReminderDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START_LOOP, &CToastReminderDlg::OnClickedStartLoop)
	ON_BN_CLICKED(IDOK, &CToastReminderDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CToastReminderDlg message handlers

BOOL CToastReminderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CToastReminderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CToastReminderDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CToastReminderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::basic_string<TCHAR> NowAsString()
{
	std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	TCHAR buf[26];
	errno_t err;
	err = _wctime_s(buf, 26, &t);

	std::basic_string<TCHAR> ts(buf); // convert to calendar time
	ts.resize(ts.size() - 1); // skip trailing newline
	return ts;
}

void ShowTimeToast(std::basic_string<TCHAR> str = {})
{
	ToastParams params;

	if (!str.empty())
		params.vectLines.push_back(str);

	auto tNow     = std::chrono::system_clock::now();
	auto to15mins = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<900>>> (tNow.time_since_epoch()) + 15min;
	auto toHours  = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<3600>>>(tNow.time_since_epoch());

	int nQuater = (to15mins - toHours) / 15min;

	switch (nQuater)
	{
	case 4:	 [[fallthrough]]
	case 0:
		params.vectLines.push_back(_T(":00"));
		params.imagePath = _T("00.png");
		break;
	case 1:
		params.vectLines.push_back(_T(":15"));
		params.imagePath = _T("15.png");
		break;
	case 2:
		params.vectLines.push_back(_T(":30"));
		params.imagePath = _T("30.png");
		break;
	case 3:
		params.vectLines.push_back(_T(":45"));
		params.imagePath = _T("45.png");
		break;
	default:
		params.vectLines.push_back(_T("Neshto se precaka!!!"));
		break;
	}

	params.vectLines.push_back(NowAsString());

	DisplayToast(params);
}


void MainThreadLoop() 
{
	do 
	{
		ToastParams params;
		auto tNow     = std::chrono::system_clock::now();
		auto to15mins = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<900>>> (tNow.time_since_epoch()) + 15min;
		auto toHours  = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<3600>>>(tNow.time_since_epoch());

#ifdef _DEBUG
		std::this_thread::sleep_for(15s);
#else
		std::this_thread::sleep_for(to15mins - tNow.time_since_epoch());
#endif

		int nQuater = (to15mins - toHours) / 15min;

		switch (nQuater)
		{
		case 4:	 [[fallthrough]]
		case 0:
			params.vectLines.push_back(_T(":00"));
			params.imagePath = _T("00.png");
			break;
		case 1:
			params.vectLines.push_back(_T(":15"));
			params.imagePath = _T("15.png");
			break;
		case 2:
			params.vectLines.push_back(_T(":30"));
			params.imagePath = _T("30.png");
			break;
		case 3:
			params.vectLines.push_back(_T(":45"));
			params.imagePath = _T("45.png");
			break;
		default:
			params.vectLines.push_back(_T("Neshto se precaka!!!"));
			break;
		}

		params.vectLines.push_back(NowAsString());

		DisplayToast(params);
	} while (true);
}


void CToastReminderDlg::OnClickedStartLoop()
{
	if (!bRunning)
	{
		ToastParams params;
		params.vectLines.emplace_back(_T("Its ON!"));
		params.vectLines.push_back(NowAsString());
		params.imagePath = _T("glavata.jpg");
		params.audioPath = _T("1.mp3");

		DisplayToast(params);


		std::thread loopThread(MainThreadLoop);
		loopThread.detach();
	}
	else
		ShowTimeToast(_T("Next toaster:"));

	ShowWindow(SW_MINIMIZE);
	bRunning = true;
}


void CToastReminderDlg::OnBnClickedOk()
{
	ShowWindow(SW_MINIMIZE);
}
