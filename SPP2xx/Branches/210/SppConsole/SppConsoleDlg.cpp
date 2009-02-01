// SppConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "..\\GlobalData\\GlobalData.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg dialog

CSppConsoleDlg::CSppConsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSppConsoleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSppConsoleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSppConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSppConsoleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSppConsoleDlg, CDialog)
	//{{AFX_MSG_MAP(CSppConsoleDlg)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_MOD_TYPE, OnSelchangeModType)
	ON_BN_CLICKED(IDC_SHIFT_AUTO, OnShiftAuto)
	ON_BN_CLICKED(IDC_SHIFT_POS, OnShiftPos)
	ON_BN_CLICKED(IDC_SHIFT_NEG, OnShiftNeg)
	ON_BN_CLICKED(IDC_HIDE, OnHide)
	ON_LBN_SELCHANGE(IDC_AUDIO_SRC, OnSelchangeAudioSrc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg message handlers

BOOL CSppConsoleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	PopulateModulation();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSppConsoleDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSppConsoleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSppConsoleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSppConsoleDlg::OnSelchangeModType() 
{
	/* Get the List Box */
	CListBox* ModTypeList = (CListBox*)GetDlgItem(IDC_MOD_TYPE);
	if (!ModTypeList)
		return ;

	/* Get the selected item */
	CString text;
	int sel = ModTypeList->GetCurSel();
	ModTypeList->GetText(sel, text);

	/* Record the selected item in the Global memory area or the Registry */
	SetActiveMode((LPCTSTR)text);	
	
}

void CSppConsoleDlg::OnShiftAuto() 
{
	/* Get the Check Box */
	CButton* AutoDetectCB = (CButton *)GetDlgItem(IDC_SHIFT_AUTO);
	if (!AutoDetectCB)
		return ;

	/* Get Status */
	int sel = AutoDetectCB->GetCheck();

	/* Record the auto detect state in the Global memory area or the Registry */ 
	SetShiftAutoDetect(sel);
	
	/* Temporary - If Auto-Detect then hide the radio buttons */
	if (sel)
		ShowShiftRB(false);
	else
		ShowShiftRB();

}

void CSppConsoleDlg::OnShiftPos() 
{
	SetPositiveShift(1);	
}


void CSppConsoleDlg::OnShiftNeg() 
{
	SetPositiveShift(0);	
}

void CSppConsoleDlg::OnHide() 
{
	// TODO: Add your control notification handler code here
	
}

void CSppConsoleDlg::OnSelchangeAudioSrc() 
{
}

/* Get the list of Modulation types in their displayable mode */
int CSppConsoleDlg::PopulateModulation()
{

	/* Get the list from the Global memory area or the Registry */
	struct Modulations *  Modulation= GetModulation(1);
	if (!Modulation || Modulation->Active <0 )
		return -1;

	/* Get the List Box */
	int selected;
	CListBox* ModTypeList = (CListBox*)GetDlgItem(IDC_MOD_TYPE);
	if (!ModTypeList)
		return -1;

	/* Loop on the list and populate the list box */
	int i=0;
	while (Modulation->ModulationList[i])
	{
		ModTypeList->InsertString(i, (Modulation->ModulationList[i])->ModTypeDisplay);
		i++;
	};
	
	/* Get the selected modulation */
	if (Modulation && Modulation->Active >= 0)
		selected = Modulation->Active;
	else
		selected = -1;

	/* Mark it as selected */
	ModTypeList->SetCurSel(selected);

	/* Shift controls */
	CButton* AutoDetectCB = (CButton *)GetDlgItem(IDC_SHIFT_AUTO);
	AutoDetectCB->SetCheck(Modulation->ShiftAutoDetect);
	CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
	CButton * NegativeRB = (CButton *)GetDlgItem(IDC_SHIFT_NEG);
	PositiveRB->SetCheck(Modulation->PositiveShift);
	NegativeRB->SetCheck(!Modulation->PositiveShift);

	/* Temporary - If Auto-Detect then hide the radio buttons */
	if (Modulation->ShiftAutoDetect)
		ShowShiftRB(false);
	else
		ShowShiftRB();
	return selected;
}

/* Get the Shift data */
//DEL void CSppConsoleDlg::PopulateShiftControls()
//DEL {
//DEL 	/* Get data from the Global memory area or the Registry */
//DEL 	/* Get the check Box */
//DEL 	/* Set value in check Box */
//DEL 	/* Get the Radio buttons */
//DEL 	/* Set value in the Radio buttons */
//DEL }

void CSppConsoleDlg::ShowShiftRB(bool show)
{
		CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
		CButton * NegativeRB = (CButton *)GetDlgItem(IDC_SHIFT_NEG);

		if (show)
		{
			PositiveRB->ShowWindow(SW_SHOW);
			NegativeRB->ShowWindow(SW_SHOW);
		}
		else
		{
			PositiveRB->ShowWindow(SW_HIDE);
			NegativeRB->ShowWindow(SW_HIDE);
		};

}
