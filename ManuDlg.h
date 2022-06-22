#pragma once


// CManuDlg dialog

class CManuDlg : public CDialog
{
	DECLARE_DYNAMIC(CManuDlg)

public:
	CManuDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CManuDlg();

// Dialog Data
	enum { IDD = IDD_MANUALCTR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
