// DefVolumeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sppconsole.h"
#include "DefVolumeDlg.h"
#include "AudioInput.h"
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefVolumeDlg dialog


CDefVolumeDlg::CDefVolumeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDefVolumeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDefVolumeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_volMic=-1;
	m_volLin=-1;
	m_volAux=-1;
	m_volAnl=-1;
}


void CDefVolumeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDefVolumeDlg)
	DDX_Control(pDX, IDC_SLIDER_ANL, m_sldrAnl);
	DDX_Control(pDX, IDC_SLIDER_AUX, m_sldrAux);
	DDX_Control(pDX, IDC_SLIDER_LIN, m_sldrLin);
	DDX_Control(pDX, IDC_EDT_LIN, m_edtLin);
	DDX_Control(pDX, IDC_EDT_AUX, m_edtAux);
	DDX_Control(pDX, IDC_EDT_ANL, m_edtAnl);
	DDX_Control(pDX, IDC_EDT_MIC, m_edtMic);
	DDX_Control(pDX, IDC_SLIDER_MIC, m_sldrMic);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDefVolumeDlg, CDialog)
	//{{AFX_MSG_MAP(CDefVolumeDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDT_MIC, OnChangeEdtMic)
	ON_EN_CHANGE(IDC_EDT_LIN, OnChangeEdtLin)
	ON_EN_CHANGE(IDC_EDT_AUX, OnChangeEdtAux)
	ON_EN_CHANGE(IDC_EDT_ANL, OnChangeEdtAnl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefVolumeDlg message handlers


/*
	One of the sliders changed position
*/
void CDefVolumeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{

	int id;
	CString Value;

	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		id = pScrollBar->GetDlgCtrlID();
		Value.Format("%d", nPos);

		// Microphone
		if (id == IDC_SLIDER_MIC)
		{
			m_edtMic.SetWindowText(Value);
			m_volMic = nPos;
		} else

		// Line In
		if (id == IDC_SLIDER_LIN)
		{
			m_edtLin.SetWindowText(Value);
			m_volLin = nPos;
		} else

		// Aux
		if (id == IDC_SLIDER_AUX)
		{
			m_edtAux.SetWindowText(Value);
			m_volAux = nPos;
		} else

		// Analog
		if (id == IDC_SLIDER_ANL)
		{
			m_edtAnl.SetWindowText(Value);
			m_volAnl = nPos;
		};
	};

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


BOOL CDefVolumeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	char tmp[6];
	
	// Limit the input text to 3 chacters
	m_edtMic.SetLimitText(3);
	m_edtLin.SetLimitText(3);
	m_edtAux.SetLimitText(3);
	m_edtAnl.SetLimitText(3);

	GetDefaultVolumeValues();// Get initial volume values

	// Set initial values to controls
	m_sldrMic.SetPos(m_volMic);
	m_sldrLin.SetPos(m_volLin);
	m_sldrAux.SetPos(m_volAux);
	m_sldrAnl.SetPos(m_volAnl);
	m_edtMic.SetWindowText(itoa(m_volMic,tmp,10));
	m_edtLin.SetWindowText(itoa(m_volLin,tmp,10));
	m_edtAux.SetWindowText(itoa(m_volAux,tmp,10));
	m_edtAnl.SetWindowText(itoa(m_volAnl,tmp,10));

	// Decorate the sliders
	m_sldrMic.SetTicFreq(25);
	m_sldrLin.SetTicFreq(25);
	m_sldrAux.SetTicFreq(25);
	m_sldrAnl.SetTicFreq(25);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
	Detected - Change in edit box next to a slider
	If the user enters a number to an edit box this method is called by the handler
	It gets the text and corrects it if it is invalid:
		Empty -> 0
		Negative -> 0
		Over 100 -> 100
	If the value has changed, the slider is moved to the new value.

	Return:
	OK:		New value in the range 0-100 
	BAD:	-1
*/
int CDefVolumeDlg::ChangeEdt(int Volume, CEdit *edt, CSliderCtrl *slider)
{
	CString  tmp;
	int val, newval;

	if (!edt || !slider)
		return -1;

	// Get the text. If no text then put 0 and exit.
	edt->GetWindowText(tmp);
	if (!tmp.GetLength())
	{
		edt->SetWindowText("0");
		return -1;
	};

	// Convert text to integer. If out of scope - put in scope
	sscanf(tmp,"%d", &val);
	newval = val;

	if (val<0)
		val=0;
	if (val>100)
		val = 100;

	// If value was out of scope, put it in edit box and move caret to end
	if (newval != val)
	{
		tmp.Format("%d",val);
		edt->SetWindowText(tmp);
		edt->SetSel(3,3);
	};

	// If value changed - update slider position
	if (val != Volume)
	{
		Volume = val;
		slider->SetPos(val);
	};

	return Volume;
}

/* Change in Mic edit box */
void CDefVolumeDlg::OnChangeEdtMic() 
{
	m_volMic = ChangeEdt(m_volMic, &m_edtMic, &m_sldrMic);
}

/* Change in Line-In edit box */
void CDefVolumeDlg::OnChangeEdtLin() 
{
	m_volLin = ChangeEdt(m_volLin, &m_edtLin, &m_sldrLin);	
}

/* Change in Aux edit box */
void CDefVolumeDlg::OnChangeEdtAux() 
{
	m_volAux = ChangeEdt(m_volAux, &m_edtAux, &m_sldrAux);	
}

/* Change in Analog edit box */
void CDefVolumeDlg::OnChangeEdtAnl() 
{
	m_volAnl = ChangeEdt(m_volAnl, &m_edtAnl, &m_sldrAnl);	
}

void CDefVolumeDlg::GetDefaultVolumeValues()
{
	m_volMic = ::GetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
	m_volLin = ::GetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_LINE);
	m_volAux = ::GetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY);
	m_volAnl = ::GetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_ANALOG);
}

void CDefVolumeDlg::OnOK() 
{

	// Put data in registry
	::SetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, m_volMic);
	::SetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_LINE, m_volLin);
	::SetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY, m_volAux);
	::SetDefaultVolumeValue(MIXERLINE_COMPONENTTYPE_SRC_ANALOG, m_volAnl);


	CDialog::OnOK();
}
