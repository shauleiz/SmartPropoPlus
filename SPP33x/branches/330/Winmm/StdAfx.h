// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__EC1A2254_7B31_4F0A_A247_C450CDCE2166__INCLUDED_)
#define AFX_STDAFX_H__EC1A2254_7B31_4F0A_A247_C450CDCE2166__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
//#include <mmsystem.h>

// TODO: reference additional headers your program requires here

extern long _ftol( double ); //defined by VC6 C libs
extern long _ftol2( double dblSource ) { return _ftol( dblSource ); }

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__EC1A2254_7B31_4F0A_A247_C450CDCE2166__INCLUDED_)
