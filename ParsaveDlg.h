#pragma once


//*****************************************************************
//External variable definition
//*****************************************************************
extern CString sBuilderName;
extern CString sFileName;


// CParsaveDlg dialog

class CParsaveDlg : public CDialog
{
	DECLARE_DYNAMIC(CParsaveDlg)

public:
	CParsaveDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParsaveDlg();

// Dialog Data
	enum { IDD = IDD_PARSAVE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
