// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__079B072F_DFCA_4A51_95A9_FA4F44631D5E__INCLUDED_)
#define AFX_STDAFX_H__079B072F_DFCA_4A51_95A9_FA4F44631D5E__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifndef WINVER		// Permit use of features specific to Windows 95 and Windows NT 4.0 or later.
#define WINVER 0x0400	// Change this to the appropriate value to target 
#endif                     // Windows 98 and Windows 2000 or later.
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers


//#define _WIN32_IE  0x0500
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0500
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
////#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxtempl.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

// Insert your headers here


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__079B072F_DFCA_4A51_95A9_FA4F44631D5E__INCLUDED_)
