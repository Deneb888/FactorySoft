// SerialNumDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "SerialNumDlg.h"
#include "afxdialogex.h"


// SerialNumDlg dialog

IMPLEMENT_DYNAMIC(SerialNumDlg, CDialog)

SerialNumDlg::SerialNumDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_SN_DIALOG, pParent)
	, m_SN(_T(""))
{

}

SerialNumDlg::~SerialNumDlg()
{
}

void SerialNumDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_SN);
}


BEGIN_MESSAGE_MAP(SerialNumDlg, CDialog)
END_MESSAGE_MAP()


// SerialNumDlg message handlers
