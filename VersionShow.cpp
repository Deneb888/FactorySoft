// VersionShow.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "VersionShow.h"
#include "afxdialogex.h"


// CVersionShow dialog

IMPLEMENT_DYNAMIC(CVersionShow, CDialog)

CVersionShow::CVersionShow(CWnd* pParent /*=NULL*/)
	: CDialog(CVersionShow::IDD, pParent)
{

}

CVersionShow::~CVersionShow()
{
}

void CVersionShow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVersionShow, CDialog)
END_MESSAGE_MAP()


// CVersionShow message handlers
