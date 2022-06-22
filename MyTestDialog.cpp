// MyTestDialog.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "MyTestDialog.h"
#include "afxdialogex.h"


// MyTestDialog dialog

IMPLEMENT_DYNAMIC(MyTestDialog, CDialogEx)

MyTestDialog::MyTestDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(MyTestDialog::IDD, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

	EnableAutomation();

}

MyTestDialog::~MyTestDialog()
{
}

void MyTestDialog::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CDialogEx::OnFinalRelease();
}

void MyTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(MyTestDialog, CDialogEx)
END_MESSAGE_MAP()

