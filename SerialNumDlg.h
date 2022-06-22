#pragma once


// SerialNumDlg dialog

class SerialNumDlg : public CDialog
{
	DECLARE_DYNAMIC(SerialNumDlg)

public:
	SerialNumDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SerialNumDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_SN;
};
