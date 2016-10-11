// SYHttpServerDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SYHttpServerDemo.h"
#include "SYHttpServerDemoDlg.h"
#include "SYHttpServerManger.h"
SYHttpServerManger *syHttpServerMgr;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSYHttpServerDemoDlg dialog




CSYHttpServerDemoDlg::CSYHttpServerDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSYHttpServerDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSYHttpServerDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSYHttpServerDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CSYHttpServerDemoDlg::OnBnClickedButton1)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON2, &CSYHttpServerDemoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CSYHttpServerDemoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSYHttpServerDemoDlg::OnBnClickedButton4)
END_MESSAGE_MAP()


// CSYHttpServerDemoDlg message handlers

BOOL CSYHttpServerDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	_listBox1 = (CListBox*)GetDlgItem(IDC_LIST1);	
	syHttpServerMgr = SYHttpServerManger::Instance();
	OnBnClickedButton1();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSYHttpServerDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSYHttpServerDemoDlg::OnPaint()
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
HCURSOR CSYHttpServerDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSYHttpServerDemoDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	WCHAR path[MAX_PATH];
	ZeroMemory(path, MAX_PATH);
	GetModuleFileName((HMODULE)GetWindowLong((HWND)GetSafeHwnd(), 0), path, MAX_PATH);
	for (size_t i = wcslen(path); path[i] !='\\'; i--) path[i] = NULL;


	syHttpServerMgr->InitServerWithPort(5000, path);
	
}

void CSYHttpServerDemoDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	delete 	syHttpServerMgr;
}

void CSYHttpServerDemoDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	syHttpServerMgr->GetOnlineClient();
}

void CSYHttpServerDemoDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CFileDialog FileDialog(TRUE,
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT,
		L"All Files: (*.*)|*.*||", NULL);


	if(FileDialog.DoModal() == IDOK)
	{
		POSITION pos = FileDialog.GetStartPosition();

		if(pos)
		{
			CString PathName;

			do
			{
				PathName = FileDialog.GetNextPathName(pos);
				// Do something with 'PathName'				
				_listBox1->AddString(PathName);
				OutputDebugString(PathName);
				WCHAR  wszPath[512];
				lstrcpy(wszPath, PathName);

				syHttpServerMgr->AddShareWithFilePath(wszPath);
			} while(pos);		
		}	
	}
}


void CSYHttpServerDemoDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	int index = _listBox1->GetCurSel();		
	if (index ==-1) {
		MessageBox(L"Please select a device.", L"Alert", MB_OK);	
		return;
	}

	CString cstr;
	_listBox1->GetText(index, cstr);
	WCHAR *pwszPath=cstr.GetBuffer(0);
	syHttpServerMgr->RemoveShareWithFilePath(pwszPath);
	_listBox1->DeleteString(index);  
}
