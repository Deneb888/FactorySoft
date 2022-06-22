#pragma once


// MyTestDialog dialog

class MyTestDialog : public CDialogEx
{
	DECLARE_DYNAMIC(MyTestDialog)

public:
	MyTestDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~MyTestDialog();

	virtual void OnFinalRelease();

// Dialog Data
	enum { IDD = IDD_MYTESTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};
