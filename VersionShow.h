#pragma once


// CVersionShow dialog

class CVersionShow : public CDialog
{
	DECLARE_DYNAMIC(CVersionShow)

public:
	CVersionShow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVersionShow();

// Dialog Data
	enum { IDD = IDD_VERSION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
