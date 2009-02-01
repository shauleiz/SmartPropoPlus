// DynamicLED.cpp : implementation file
//

#include "stdafx.h"
#include "DynamicLED.h"

#define ID_TIMER_START 1001

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynamicLED

CDynamicLED::CDynamicLED()
{
	// Initialize the variables
	m_bBright = FALSE;
	m_nTimerInterval = 0;
	m_nPrevTimerInterval = 0;
	m_pdc = NULL;
	m_BlinkTimer = 0;
	m_hWnd = NULL;
}

CDynamicLED::~CDynamicLED()
{
	if (m_pdc) delete m_pdc;
}

BEGIN_MESSAGE_MAP(CDynamicLED, CStatic)
	//{{AFX_MSG_MAP(CDynamicLED)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynamicLED message handlers

/*
	Initialize led to  shape & colour
	Set of Off
*/
void CDynamicLED::InitLed(CWnd *pWnd, UINT nIDColor, UINT nIDShape)
{
	m_hWnd = pWnd->m_hWnd;

	// Save the color,shape and the timer interval of the control as it 
	// will be used later when resetting the LED to its previous state.
	m_nID = nIDColor;
	m_nShape = nIDShape;

	if (!m_pdc) m_pdc = new CClientDC(this);
	SetPenBrush();

	SwitchOff();
	return;
}

void CDynamicLED::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	Switch(m_bBright);
	return;
}

void CDynamicLED::OnTimer(UINT nIDEvent) 
{	
	SetLedBright(NULL,TRUE);

	// Get the rectangle of the window where we are going to draw
	CRect rcClient;
	GetClientRect(&rcClient);

	// Draw the LED (Round/Square)
	if(m_nShape==ID_SHAPE_ROUND) DrawRoundLed(rcClient);
	else if(m_nShape==ID_SHAPE_SQUARE) DrawSquareLed(rcClient);

	CStatic::OnTimer(nIDEvent);
}

void CDynamicLED::Switch(BOOL on)
{
	if (m_BlinkTimer) 
	{
		KillTimer(m_BlinkTimer);
		m_BlinkTimer = 0;
	};

	SetLedBright(on,FALSE);

	// Get the rectangle of the window where we are going to draw
	CRect rcClient;
	GetClientRect(&rcClient);

	// Draw the LED (Round/Square)
	if(m_nShape==ID_SHAPE_ROUND) DrawRoundLed(rcClient);
	else if(m_nShape==ID_SHAPE_SQUARE) DrawSquareLed(rcClient);

	return;
}

void CDynamicLED::SwitchOn()
{
	Switch(TRUE);
}

void CDynamicLED::SwitchOff()
{
	Switch(FALSE);
}

void CDynamicLED::Blink(int milisec)
{
	static rate=0;

	if (!rate)
		rate = milisec;
	else if (m_BlinkTimer && milisec == rate)
		return;

	if (m_BlinkTimer) KillTimer(m_BlinkTimer);
	m_BlinkTimer = SetTimer(ID_TIMER_START,milisec,NULL);
}

/*
	Set LED to Bright/Dark if 'Toggle'=FALSE (Default)
	If Toggle=TRUE then toggle brightness (Ignor value of 'Bright')
*/
BOOL CDynamicLED::SetLedBright(BOOL Bright, BOOL Toggle)
{
	if (!m_pdc)
		return FALSE;

	// Save previous value
	BOOL ret = m_bBright;

	// Set brightness
	if (Toggle && m_bBright) m_bBright = FALSE;
	else if (Toggle && !m_bBright) m_bBright = TRUE;
	else if (Bright) m_bBright = TRUE;
	else m_bBright = FALSE;

	if(m_bBright==TRUE)
	{
		// If we have to switch on the LED to display the bright colour select
		// the bright pen and brush that we have created above

		m_pdc->SelectObject(&m_PenBright);
		m_pdc->SelectObject(&m_BrushBright);

		m_BrushCurrent.m_hObject = m_BrushBright.m_hObject;
	}
	else
	{
		// If we have to switch off the LED to display the dark colour select
		// the bright pen and brush that we have created above

		m_pdc->SelectObject(&m_PenDark);
		m_pdc->SelectObject(&m_BrushDark);

		m_BrushCurrent.m_hObject = m_BrushDark.m_hObject;
	}

	return ret;
}

void CDynamicLED::SetPenBrush(void)
{
	// If the pen has been selected already, then we have to delete it
	// so that it doesnt throw an assertion

	if(m_PenBright.m_hObject!=NULL)
		m_PenBright.DeleteObject();

	if(m_BrushBright.m_hObject!=NULL)
		m_BrushBright.DeleteObject();

	if(m_PenDark.m_hObject!=NULL)
		m_PenDark.DeleteObject();

	if(m_BrushDark.m_hObject!=NULL)
		m_BrushDark.DeleteObject();	

	// If the user has selected RED as the color of the LED
	if(m_nID==ID_LED_RED)
	{
		// I'm creating a light shade of red here for displaying the bright
		// LED. You can change the values to any colour that you want
		m_PenBright.CreatePen(0,1,RGB(250,0,0));
		m_BrushBright.CreateSolidBrush(RGB(250,0,0));

		// Here i'm creating a dark shade of red. You can play with the values to
		// see the effect on the LED control

		m_PenDark.CreatePen(0,1,RGB(150,0,0));
		m_BrushDark.CreateSolidBrush(RGB(150,0,0));
	}
	else if(m_nID==ID_LED_GREEN)
	{
		// If the user has selected GREEN as the color of the LED

		m_PenBright.CreatePen(0,1,RGB(0,250,0));
		m_BrushBright.CreateSolidBrush(RGB(0,250,0));

		m_PenDark.CreatePen(0,1,RGB(0,150,0));
		m_BrushDark.CreateSolidBrush(RGB(0,150,0));
	}
	else if(m_nID==ID_LED_BLUE)
	{
		// If the user has selected BLUE as the color of the LED

		m_PenBright.CreatePen(0,1,RGB(0,0,250));
		m_BrushBright.CreateSolidBrush(RGB(0,0,250));

		m_PenDark.CreatePen(0,1,RGB(0,0,150));
		m_BrushDark.CreateSolidBrush(RGB(0,0,150));
	}
	else if(m_nID==ID_LED_YELLOW)
	{
		// If the user has selected YELLOW as the color of the LED

		m_PenBright.CreatePen(0,1,RGB(200,200 ,0));
		m_BrushBright.CreateSolidBrush(RGB(200,200,0));

		m_PenDark.CreatePen(0,1,RGB(150,150,0));
		m_BrushDark.CreateSolidBrush(RGB(150,150,0));
	}

}
void CDynamicLED::DrawRoundLed(CRect rcClient)
{
	//// Get the rectangle of the window where we are going to draw
	//CRect rcClient;
	//GetClientRect(&rcClient);

	if (!m_pdc)
		return;

	// Draw the actual colour of the LED
	m_pdc->Ellipse(rcClient);

	// Draw a thick dark gray coloured circle
	CPen Pen;
	Pen.CreatePen(0,2,RGB(128,128,128));
	m_pdc->SelectObject(&Pen);

	m_pdc->Ellipse(rcClient);

	// Draw a thin light gray coloured circle
	Pen.DeleteObject();
	Pen.CreatePen(0,1,RGB(192,192,192));
	m_pdc->SelectObject(&Pen);
	m_pdc->Ellipse(rcClient);

	// Draw a white arc at the bottom
	Pen.DeleteObject();
	Pen.CreatePen(0,1,RGB(255,255,255));
	m_pdc->SelectObject(&Pen);

	// The arc function is just to add a 3D effect for the control
	CPoint ptStart,ptEnd;
	ptStart = CPoint(rcClient.Width()/2,rcClient.bottom);
	ptEnd	= CPoint(rcClient.right,rcClient.top);

	m_pdc->MoveTo(ptStart);
	m_pdc->Arc(rcClient,ptStart,ptEnd);

	CBrush Brush;
	Brush.CreateSolidBrush(RGB(255,255,255));
	m_pdc->SelectObject(&Brush);

	// Draw the actual ellipse
	m_pdc->Ellipse(rcClient.left+4,rcClient.top+4,rcClient.left+6,rcClient.top+6);
}

void CDynamicLED::DrawSquareLed(CRect rcClient)
{
	if (!m_pdc)
		return;

	// Draw the actual rectangle
	m_pdc->FillRect(rcClient,&m_BrushCurrent);

	// The  code below gives a 3D look to the control. It does nothing more

	// Draw the dark gray lines
	CPen Pen;
	Pen.CreatePen(0,1,RGB(128,128,128));
	m_pdc->SelectObject(&Pen);

	m_pdc->MoveTo(rcClient.left,rcClient.bottom);
	m_pdc->LineTo(rcClient.left,rcClient.top);
	m_pdc->LineTo(rcClient.right,rcClient.top);

	Pen.DeleteObject();

	// Draw the light gray lines
	Pen.CreatePen(0,1,RGB(192,192,192));
	m_pdc->SelectObject(&Pen);

	m_pdc->MoveTo(rcClient.right,rcClient.top);
	m_pdc->LineTo(rcClient.right,rcClient.bottom);
	m_pdc->LineTo(rcClient.left,rcClient.bottom);	
}

