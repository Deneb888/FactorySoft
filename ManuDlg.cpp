// ManuDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "ManuDlg.h"
#include "afxdialogex.h"


// CManuDlg dialog

IMPLEMENT_DYNAMIC(CManuDlg, CDialog)

CManuDlg::CManuDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CManuDlg::IDD, pParent)
{

}

CManuDlg::~CManuDlg()
{
}

void CManuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CManuDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CManuDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CManuDlg message handlers


void CManuDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
