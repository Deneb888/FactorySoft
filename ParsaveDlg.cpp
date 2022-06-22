// ParsaveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "ParsaveDlg.h"
#include "afxdialogex.h"


//***************************************************************
//Global variable definition
//***************************************************************
CString sBuilderName;
CString sFileName;		


// CParsaveDlg dialog

IMPLEMENT_DYNAMIC(CParsaveDlg, CDialog)

CParsaveDlg::CParsaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParsaveDlg::IDD, pParent)
{

}

CParsaveDlg::~CParsaveDlg()
{
}

void CParsaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CParsaveDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CParsaveDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CParsaveDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CParsaveDlg message handlers


void CParsaveDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
//	CDialog::OnOK();

	sBuilderName.Empty();
	sFileName.Empty();
	GetDlgItemTextA(IDC_EDIT_PARSAVE_NAME,sBuilderName);
	GetDlgItemTextA(IDC_EDIT_PARSAVE_FILENAME,sFileName);

	WPARAM a=8;
	LPARAM b=9;
	HWND hWnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hWnd,WM_ParsaveDlg_event,a,b);

	CDialog::OnOK();
}


void CParsaveDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
