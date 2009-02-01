// AboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sppconsole.h"
#include "AboutDlg.h"
#include "SmartPropoPlus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog


CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}


void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg message handlers
BOOL CAboutDlg::OnInitDialog() 
{
		
	/* Link to homepage */
	m_homesite.SubclassDlgItem(IDC_WEB_LINK, this);
	m_homesite.SetWindowText(SPP_HOME_TITLE);
	m_homesite.SetLink(SPP_HOME_URL);

	/* Main pane text */
	CString MainText ;
	int VerHi, VerMid, VerLo;
	CWnd* ModTypeList = GetDlgItem(IDC_TTL_2);
	// GUI Version
	VerHi  = (VER_GUI&0xFF0000)>>16;
	VerMid = (VER_GUI&0x00FF00)>>8;
	VerLo  = (VER_GUI&0x0000FF)>>0;
	// Compilation date
	CTime t = CTime::GetCurrentTime();
	CString date = t.Format("%d%b%Y");

	MainText.Format(SPP_MAIN_TEXT,VerHi,VerMid,VerLo, date);
	ModTypeList->SetWindowText(MainText);

	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
