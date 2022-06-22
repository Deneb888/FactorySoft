#pragma once


// IntTimeDlg dialog

class IntTimeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(IntTimeDlg)

public:
	IntTimeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~IntTimeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	float m_IntTime;
//	afx_msg void OnBnClickedButtonNew();
	// New experiment name
//	CString m_ExpName;
	afx_msg void OnBnClickedButton2();
	CString m_ExpName2;
	afx_msg void OnBnClickedButton13();
	afx_msg void OnClickedButton1();
	CString m_trimName;
	CString m_trimName2;
	CString m_trimName3;
	CString m_trimName4;
	afx_msg void OnClickedButton12();
	afx_msg void OnBnClickedButton14();
};
