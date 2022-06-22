#pragma once
#include "afxwin.h"

//***************************************************************
//Constant definition
//***************************************************************
#define UM_TRIMPROCESS WM_USER+401	//Trim dialog自定义消息序号

#define SENDTRIMMSG		1		// ReadRow button
#define TRIM_TWO_HID	2		// set data to both of two HID
#define	EEPROMMSG		3		// For EEProm read, need to read multi packets
#define trimdatanum		200		// graphic dialog transmitted data number

extern BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

extern int PCRMask;					// chan mask

typedef struct
{
	int chipnum;
	unsigned char rampgen_val;
	float adc_trim[24];
	float lowgain[12];
	float highgain[12];
	float temp_trim[2];
}TRIM_DATA;

// CTrimDlg dialog

class CTrimDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrimDlg)

public:
	CTrimDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrimDlg();

// Dialog Data
	enum { IDD = IDD_TRIM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnReread();
	void TrimCalMainMsg();
	afx_msg LRESULT OnTrimProcess(WPARAM wParam, LPARAM lParam);
	CComboBox m_ComboxRampgen;
	virtual BOOL OnInitDialog();
	CComboBox m_ComboxRange;
	CComboBox m_ComboxAmux;
	CComboBox m_ComboxIpix;
	CComboBox m_ComboxSwitch;
	CComboBox m_ComboxTestADC;
	CComboBox m_ComboxTX;
	CComboBox m_ComboxV15;
	CComboBox m_ComboxV20;
	CComboBox m_ComboxV24;
	CEdit m_EditRecData;
	CEdit m_EditData;
	afx_msg void OnCbnSelchangeComboRampgen();
	afx_msg void OnCbnSelchangeComboRange();
	afx_msg void OnCbnSelchangeComboV24();
	afx_msg void OnCbnSelchangeComboV20();
	afx_msg void OnCbnSelchangeComboV15();
	afx_msg void OnCbnSelchangeComboIpix();
	afx_msg void OnCbnSelchangeComboSwitch();
	afx_msg void OnCbnSelchangeComboTxbin();
	afx_msg void OnCbnSelchangeComboAmuxsel();
	afx_msg void OnCbnSelchangeComboTestadc();
	afx_msg void OnBnClickedBtnSendcmd();
	afx_msg void OnBnClickedBtnItgtim();
	void ResetTrim(void);
	void SetRange(char range);
	void SetV20(char v20);
	void SetRampgen(char rampgen);
	void SetTXbin(char txbin);
	void ResetTxBin(void);
	afx_msg void OnBnClickedTrimBtnPcrtemp1();
	afx_msg void OnBnClickedTrimBtnPcrtemp2();
	afx_msg void OnBnClickedBtnShowdata();
	int m_radio_trimdlg_isensor;
	BYTE SensorRadioSel(BYTE selBuf);
	afx_msg void OnClickedRadioTrimSensor1();
	afx_msg void OnBnClickedRadioTrimSensor2();
	afx_msg void OnBnClickedRadioTrimSensor3();
	afx_msg void OnBnClickedRadioTrimSensor4();
	afx_msg void OnBnClickedTrimBtnLoad();
	afx_msg void OnClickedTrimBtnTest();

	void SelSensor(BYTE);
	void SetV20(char V20, char i);
	void SetV15(char v15);
	void SetIntTime(float kfl, int ch);
	afx_msg void OnBnClickedBtnItgtim2();
	afx_msg void OnBnClickedBtnItgtim3();
	afx_msg void OnBnClickedBtnItgtim4();
	afx_msg void OnCbnSelchangeComboTxbin2();
	afx_msg void OnCbnSelchangeComboTxbin3();
	afx_msg void OnCbnSelchangeComboTxbin4();

	void SetGainMode(int);
	void SetGainMode(int, char);
	void SetLEDConfig(BOOL IndvEn, BOOL Chan1, BOOL Chan2, BOOL Chan3, BOOL Chan4);

	CComboBox m_ComboxTX2;
	CComboBox m_ComboxTX3;
	CComboBox m_ComboxTX4;

	void TrimGetPCRMaskStatus();
//	int m_GainMode;
//	afx_msg void OnBnClickedRadioLowgain();
//	afx_msg void OnBnClickedRadioHighGain();
	void CommitTrim();
//	int m_GainMode;
	afx_msg void OnBnClickedRadioLowgain();
	afx_msg void OnBnClickedRadioHighGain();
//	int m_GainMode;
	int m_GainMode;
	BOOL m_FrameSize;
	afx_msg void OnBnClickedRadioFrame12();
	afx_msg void OnBnClickedRadioFrame24();
	void SetFrameSizeFromGUI();
	void ResetParams();
	afx_msg void OnKillfocusTrimEditIntergratetime();
	afx_msg void OnKillfocusTrimEditIntergratetime2();
	afx_msg void OnKillfocusTrimEditIntergratetime3();
	afx_msg void OnKillfocusTrimEditIntergratetime4();

	void SetIntTimeEdit1(CString);
	void SetIntTimeEdit2(CString);
	void SetIntTimeEdit3(CString);
	void SetIntTimeEdit4(CString);
	void SetMGainMode(int);
	void SetPCRMask(int);

	float DynamicUpdateIntTime(float factor, int chan);

	void SetIntTimeGUI(CString, int);

	void EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size);
	void EEPROMRead(int chan);
	afx_msg void OnBnClickedButtonWreep();
	afx_msg void OnBnClickedButtonRdeep();
	CString ReadFlash();
	afx_msg void OnBnClickedButtonVerify();
//	afx_msg void OnKillfocusTrimEditPcrtemp1();
//	afx_msg void OnTrimEditTempcompen55();
	afx_msg void OnEnKillfocusTrimEditTempcompen95();
	afx_msg void OnEnKillfocusTrimEditTempcompen55();

	float m_tc95, m_tc55;
	afx_msg void OnBnClickedTrimBtnSn();
	afx_msg void OnBnClickedButtonSavef();
	afx_msg void OnBnClickedButtonReadf();
};
