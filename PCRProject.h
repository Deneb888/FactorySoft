
// PCRProject.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CPCRProjectApp:
// See PCRProject.cpp for the implementation of this class
//

class CPCRProjectApp : public CWinApp
{
public:
	CPCRProjectApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CPCRProjectApp theApp;