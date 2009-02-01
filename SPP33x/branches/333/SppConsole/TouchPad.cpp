// TouchPad.cpp : implementation file
//

#include "stdafx.h"
#include "sppconsole.h"
#include "TouchPad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTouchPad

IMPLEMENT_DYNAMIC(CTouchPad, CStatic)

BEGIN_MESSAGE_MAP(CTouchPad, CStatic)
	//{{AFX_MSG_MAP(CTouchPad)
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
CTouchPad::CTouchPad()
{
}

CTouchPad::~CTouchPad()
{
}

/////////////////////////////////////////////////////////////////////////////
// CTouchPad message handlers

UINT CTouchPad::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	UINT res = CStatic::OnNcHitTest(point);
	if (res != HTCLIENT)
		::Beep(1500,100);
	return res;
	   
}
