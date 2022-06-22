
// PCRProject.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"

#include "SystemConfig.cpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPCRProjectApp

BEGIN_MESSAGE_MAP(CPCRProjectApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPCRProjectApp construction

CPCRProjectApp::CPCRProjectApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CPCRProjectApp object

CPCRProjectApp theApp;


// CPCRProjectApp initialization

BOOL CPCRProjectApp::InitInstance()
{
	AfxOleInit();
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	const char szUniqueNamedMutex[] = "com_anitoa_pcrproject_alpha";
	HANDLE hHandle = CreateMutex(NULL, TRUE, szUniqueNamedMutex);
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		// Program already running somewhere

		//added by lixin 20190327
		switch(currentLanguage){
			case language_English:
				MessageBoxA(NULL, "The application is already running", "PCRProject (c)", IDOK);
				break;
			case language_Chinese:
				MessageBoxA(NULL, "程序正在运行，请勿重复打开", "PCRProject (c)", IDOK);
				break;
		} 
		return(1); // Exit program
	}

	// Program runs...


	CPCRProjectDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Upon app closing:
	ReleaseMutex(hHandle); // Explicitly release mutex
	CloseHandle(hHandle); // close handle before terminating
	//return(1);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

