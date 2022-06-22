#pragma once

//***************************************************************
//Constant definition
//***************************************************************
#define UM_REGPROCESS WM_USER+501

#define sendregmsg 1
#define regdatanum 200	// graphic dialog transmitted data number

// CRegDlg dialog

class CRegDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRegDlg)

public:
	CRegDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegDlg();

// Dialog Data
	enum { IDD = IDD_REGISTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CBrush m_editbrush;

	DECLARE_MESSAGE_MAP()
public:
//	CString m_EditRowNum;
	CString m_EditKP;
	afx_msg void OnBnClickedBtnKp();
	afx_msg void OnBnClickedBtnKi();
	afx_msg void OnBnClickedBtnKd();
	afx_msg void OnBnClickedBtnKl();
	afx_msg void OnBnClickedBtnP1start();
	afx_msg void OnBnClickedBtnP1stop();
	afx_msg void OnBnClickedBtnP2start();
	afx_msg void OnBnClickedBtnP2stop();
	afx_msg void OnBnClickedBtnHtstart();
	afx_msg void OnBnClickedBtnHtstop();
	afx_msg void OnBnClickedBtnChip();
	afx_msg void OnBnClickedBtnPeltier1();
	afx_msg void OnBnClickedBtnPeltier2();
	afx_msg LRESULT OnRegProcess(WPARAM wParam,LPARAM lParam);
	void RegCalMainMsg();
	afx_msg void OnBnClickedBtnTempset();
	afx_msg void OnBnClickedBtnTempstar();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnP1clear();
	afx_msg void OnBnClickedBtnP2clear();
	afx_msg void OnBnClickedBtnP1save();
	afx_msg void OnBnClickedBtnP2save();
	afx_msg void OnBnClickedBtnFanconAuto();
	afx_msg void OnBnClickedBtnFanconOn();
	afx_msg void OnBnClickedBtnFanconOff();
	afx_msg void OnEnChangeEditPeltier2();
	afx_msg void OnBnClickedBtnFanconState();
	CString m_fanstate;
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnChangeEditFanState();
	CString m_EditKI;
	CString m_EditKL;
	afx_msg void OnBnClickedBtnFantemp();
	afx_msg void OnBnClickedBtnLedlight();
//	afx_msg void OnBnClickedBtnPidRead();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedBtnLoad();
	CString m_EditInitemp;
	CString m_EditInitim;
	CString m_EditKD;
	CString m_EditDentemp;
	CString m_EditDentim;
	CString m_EditAnntemp;
//	CEdit m_EditAnntim;
	CString m_EditAnntim;
	CString m_EditInextentemp;
	CString m_EditInextentim;
	CString m_EditExextentemp;
	CString m_EditExextentim;
	CString m_EditCyclenum;
	CString sPeltier1;
	CString sPeltier2;
//	afx_msg void OnBnClickedBtnPidRead();
	afx_msg void OnBnClickedBtnReadPid1();
};
