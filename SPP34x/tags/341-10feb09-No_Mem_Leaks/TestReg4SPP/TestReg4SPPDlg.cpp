// TestReg4SPPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestReg4SPP.h"
#include "TestReg4SPPDlg.h"
#include ".\testreg4sppdlg.h"
#include "../include/smartpropoplus.h"
#include "../include/SppRegistry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestReg4SPPDlg dialog



CTestReg4SPPDlg::CTestReg4SPPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestReg4SPPDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestReg4SPPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOGFILENAME, m_ctrl_Log_File_Name);
}

BEGIN_MESSAGE_MAP(CTestReg4SPPDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CTestReg4SPPDlg message handlers

BOOL CTestReg4SPPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_pLogFile = OpenLogfile(&m_LogPath);
	if (m_pLogFile)
		m_ctrl_Log_File_Name.SetWindowText(m_LogPath);
	else
		m_ctrl_Log_File_Name.SetWindowText("<Error>");

	ShowWindow(SW_NORMAL);

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestReg4SPPDlg::OnPaint() 
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
HCURSOR CTestReg4SPPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestReg4SPPDlg::OnBnClickedOk()
{
	if (!m_pLogFile)
		OnOK();

	LPVOID lpMsgBuf;

	// Enter header to the log file
	char dbuffer [9], tbuffer [9];
	fprintf(m_pLogFile,"*******************  Registry Test Utility  *******************\n");
	fprintf(m_pLogFile, "%s - %s\n", _strdate( dbuffer ), _strtime( tbuffer ));

	// Test existance of FMS registry key - create if does not exist
	fprintf(m_pLogFile, "Checking FMS registry key: %s\n", REG_FMS);
	int FmsExist = isFmsRegistryExist();
	if (FmsExist)
		fprintf(m_pLogFile, "Checking FMS registry key: %s\n", "... Existed");
	else
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Checking FMS registry key: error number %d  - %s", GetLastError(), (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Creating FMS registry key: %s\n", REG_FMS);
		FmsExist = CreateEmptyFmsRegistry();
		if (FmsExist)
			fprintf(m_pLogFile, "Creating FMS registry key: %s\n", "... Created");
		else
		{
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
					fprintf(m_pLogFile, "Creating FMS registry key: error number %d  - %s", GetLastError(), (LPCTSTR)lpMsgBuf);
		};
	};
	/***********************************************************************************************************/

	// Test existance of SPP registry key - create if does not exist
	fprintf(m_pLogFile, "Checking SPP registry key: %s\n", REG_SPP);
	int SppExist = isSppRegistryExist();
	if (SppExist)
		fprintf(m_pLogFile, "Checking SPP registry key: %s\n", "... Existed");
	else
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Checking SPP registry key: error number %d  - %s", GetLastError(), (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Creating SPP registry key: %s\n", REG_SPP);
		SppExist = CreateDefaultSppRegistry();
		if (SppExist)
			fprintf(m_pLogFile, "Creating SPP registry key: %s\n", "... Created");
		else
		{
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
					fprintf(m_pLogFile, "Creating SPP registry key: error number %d  - %s", GetLastError(), (LPCTSTR)lpMsgBuf);
		};
	};
	/***********************************************************************************************************/

	int res;
	HKEY hSpp;
	int DebugLevel;
	// Debug Level section
	// Test existence & value
	fprintf(m_pLogFile, "Checking registry value: %s\n", DEBUG_LEVEL);
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE, &hSpp); // Open SPP for reading
	if (res != ERROR_SUCCESS)
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to open SPP registry key for reading : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	};

	DWORD ValueDataSize = 4;
	DWORD ValueType;
	res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, &ValueType, (unsigned char *)&DebugLevel,  &ValueDataSize);
	if (res != ERROR_SUCCESS)
	{	// Could not read DEBUG_LEVEL data
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to open registry value  %s for reading : error number %d  - %s", DEBUG_LEVEL, res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Creating registry value  %s with value 0\n",DEBUG_LEVEL);
		res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_WRITE, &hSpp); // Open SPP key for writing
		if (res != ERROR_SUCCESS)
		{	// Cannot Open SPP key for writing - stop
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(m_pLogFile, "Failed to open SPP registry key for writing : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
			fprintf(m_pLogFile, "Stopping\n");
			return;
		}
		else
			fprintf(m_pLogFile, "Registry key %s opened for writing\n", REG_SPP);

		DebugLevel = 0;
		fprintf(m_pLogFile, "Setting data %d to registry value %s\n", DebugLevel, DEBUG_LEVEL);
		res =  RegSetValueEx(hSpp, DEBUG_LEVEL  ,0, REG_DWORD, (const BYTE *)&DebugLevel, 4); // Set data in DEBUG-LEVEL 
		if (res != ERROR_SUCCESS)
		{	// Cannot set data in DEBUG-LEVEL - stopping
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(m_pLogFile, "Failed to set data to registry value : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
			fprintf(m_pLogFile, "Stopping\n");
			return;
		}
		else
			fprintf(m_pLogFile, "setting data to registry value %s ... OK\n", DEBUG_LEVEL);
		RegCloseKey(hSpp); // At this point, it is assumed that SPP is created and that DEBUG-LEVEL data set

		// Now getting the value
		fprintf(m_pLogFile, "Open registry key %s for query\n", REG_SPP);
		res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE, &hSpp); // Open SPP for reading
		if (res != ERROR_SUCCESS)
		{	// Cannot open SPP for reading - stop
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(m_pLogFile, "Failed to open SPP registry key for reading : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
			fprintf(m_pLogFile, "Stopping\n");
			return;
		}
		else
			fprintf(m_pLogFile, "Open registry key %s for query ... OK\n", REG_SPP);

		ValueDataSize = 4;
		fprintf(m_pLogFile, "Checking registry value: %s\n", DEBUG_LEVEL);
		res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, &ValueType, (unsigned char *)&DebugLevel,  &ValueDataSize); // Reading the data from DEBUG-LEVEL

		if (res != ERROR_SUCCESS)
		{ // Cannot read the DEBUG-LEVEL value - stop
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(m_pLogFile, "Failed to read data from %s : error number %d  - %s", DEBUG_LEVEL ,res, (LPCTSTR)lpMsgBuf);
			fprintf(m_pLogFile, "Stopping\n");
			return;
		}
		else
			fprintf(m_pLogFile, "Checking registry value: %s ...OK\n", DEBUG_LEVEL);
	};

	// Print the DEBUG-LEVEL value
	fprintf(m_pLogFile, "Registry value %s: data=%d (Type=%d)\n", DEBUG_LEVEL, DebugLevel, ValueType);
	RegCloseKey(hSpp); // Close the SPP key
	/***********************************************************************************************************/
	
	// Some manipulations of DEBUG-LEVEL
	fprintf(m_pLogFile, "\n************************* Testing %s ************************************\n",DEBUG_LEVEL);
	fprintf(m_pLogFile, "Open registry key %s for all access\n", REG_SPP);
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS, &hSpp); // Open SPP for all access
	if (res != ERROR_SUCCESS)
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to open key %s : error number %d  - %s", REG_SPP ,res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
	}
	else
		fprintf(m_pLogFile, "Open registry key %s for all access ... OK\n", REG_SPP);

	// Read current DEBUG-LEVEL value
	ValueDataSize = 4;
	fprintf(m_pLogFile, "Checking registry value: %s\n", DEBUG_LEVEL);
	res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, &ValueType, (unsigned char *)&DebugLevel,  &ValueDataSize); // Reading the data from DEBUG-LEVEL
	if (res != ERROR_SUCCESS)
	{ // Cannot read the DEBUG-LEVEL value - stop
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to read data from %s : error number %d  - %s", DEBUG_LEVEL ,res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	}
	else
		fprintf(m_pLogFile, "Checking registry value: %s data=%d (type=%d)\n", DEBUG_LEVEL, DebugLevel, ValueType);

	// change DEBUG-LEVEL value
	DebugLevel = 111;
	fprintf(m_pLogFile, "Setting data %d to registry value %s\n", DebugLevel, DEBUG_LEVEL);
	res =  RegSetValueEx(hSpp, DEBUG_LEVEL  ,0, REG_DWORD, (const BYTE *)&DebugLevel, 4); // Set data in DEBUG-LEVEL 
	if (res != ERROR_SUCCESS)
	{	// Cannot set data in DEBUG-LEVEL - stopping
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to set data to registry value : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	}
	else
		fprintf(m_pLogFile, "setting data to registry value %s ... OK\n", DEBUG_LEVEL);

	// Read current DEBUG-LEVEL value
	ValueDataSize = 4;
	fprintf(m_pLogFile, "Checking registry value: %s\n", DEBUG_LEVEL);
	res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, &ValueType, (unsigned char *)&DebugLevel,  &ValueDataSize); // Reading the data from DEBUG-LEVEL
	if (res != ERROR_SUCCESS)
	{ // Cannot read the DEBUG-LEVEL value - stop
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to read data from %s : error number %d  - %s", DEBUG_LEVEL ,res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	}
	else
		fprintf(m_pLogFile, "Checking registry value: %s data=%d (type=%d)\n", DEBUG_LEVEL, DebugLevel, ValueType);

	// Delete DEBUG-LEVEL value
	fprintf(m_pLogFile, "Deleting registry value: %s\n", DEBUG_LEVEL);
	res = RegDeleteValue(hSpp, DEBUG_LEVEL);
	if (res != ERROR_SUCCESS)
	{ // Cannot read the DEBUG-LEVEL value - stop
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to delet data  %s : error number %d  - %s", DEBUG_LEVEL ,res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	}
	else
		fprintf(m_pLogFile, "Deleting registry value: %s ... OK\n", DEBUG_LEVEL);

	// change DEBUG-LEVEL value to 3
	DebugLevel = 3;
	fprintf(m_pLogFile, "Setting data %d to registry value %s\n", DebugLevel, DEBUG_LEVEL);
	res =  RegSetValueEx(hSpp, DEBUG_LEVEL  ,0, REG_DWORD, (const BYTE *)&DebugLevel, 4); // Set data in DEBUG-LEVEL 
	if (res != ERROR_SUCCESS)
	{	// Cannot set data in DEBUG-LEVEL - stopping
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
		fprintf(m_pLogFile, "Failed to set data to registry value : error number %d  - %s", res, (LPCTSTR)lpMsgBuf);
		fprintf(m_pLogFile, "Stopping\n");
		return;
	}
	else
		fprintf(m_pLogFile, "setting data to registry value %s ... OK\n", DEBUG_LEVEL);

	/***********************************************************************************************************/
#if 0
	SECURITY_INFORMATION SecurityInfo;
	SECURITY_DESCRIPTOR	SecurityDesc;
	DWORD	size;
	PSID	Owner;
	BOOL Default;

	size = 100;
	SecurityInfo = OWNER_SECURITY_INFORMATION;
	res = RegGetKeySecurity(hSpp, SecurityInfo, &SecurityDesc, &size);
	BOOL SecurityStatus = GetSecurityDescriptorOwner(&SecurityDesc, &Owner, &Default); 

#endif



	// Closing
	RegCloseKey(hSpp); // Close the SPP key
	fclose(m_pLogFile);
	//OnOK();
}

// Open log file for writing. Return pointer to file. Put full path in 'path'
FILE * CTestReg4SPPDlg::OpenLogfile(CString * path)
{
	char chPath[1000];

	sprintf(chPath, "%s\\%s", getenv("TEMP"), REG_LOG_FILE);
	path->Format("%s",chPath);
	return fopen(chPath, "w");

}

void CTestReg4SPPDlg::OnBnClickedCancel()
{
	if (!m_pLogFile)
		fclose(m_pLogFile);
	OnCancel();
}

// Test existence of key. Create if does not exist
int CTestReg4SPPDlg::CreateKey(const char * KeyName)
{

	int FmsExist = isFmsRegistryExist();
	if (!FmsExist)
		CreateEmptyFmsRegistry();

	return 0;
}
