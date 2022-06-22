#pragma once

#include "ManuDlg.h"
#include "ParsaveDlg.h"

#include "chart\ChartCtrl\ChartLineSerie.h"
#include "chart\chartctrl\chartctrl.h"


//*****************************************************************
//Constant definition
//*****************************************************************
// operation inner message
//#define UM_OPERPROCESS WM_USER + 108			// operation dialog 自定义消息序号
#define UM_OPERPROCESS WM_USER + 301			// operation dialog 自定义消息序号
#define UM_OPERPIDPROCESS WM_USER + 302			// operation dialog PID返回数据处理消息序号
#define UM_OPERLEDPROCESS WM_USER + 303			// operation dialog LED返回数据处理消息序号
#define UM_OPEROVERSHOOTPROCESS WM_USER + 304	// operation dialog overshoot返回数据处理消息序号
#define UM_OPERCHECKPOLLING WM_USER +305		// operation dialog 发送查询是否继续polling消息序号
#define UM_OPERSAVERAMDATA WM_USER +306			// operation dialog 实验保存RAM数据

// parameter default
#define PITEMP			"105"
#define INIDENTEMP		"95"
#define INIDENTIME		"120"					// Updated for safetrace by Zhimin
#define DENTEMP			"95"
#define DENTIME			"15"
#define	DENRAMP			"5"
#define ANNTEMP			"60"
#define ANNTIME			"20"
#define	ANNRAMP			"2"
#define ANNTEMP2		"60"
#define ANNTIME2		"20"
#define	ANNRAMP2		"2"
#define EXTTEMP			"60"
#define EXTTIME			"20"
#define	EXTRAMP			"2"
#define HOLDTEMP		"50"
#define HOLDTIME		"10"
#define ANNCYCLE		"50"
#define KP				"10"
#define KI				"10"
#define KD				"10"
#define KL				"10"

// operation dialog message to main dialog
#define OPEREADSTATUS		1
#define OPESENDMSG			2
#define OPETOGRAMSG			3
#define OPEGETPCRMASK		4

// opeDlg internal const value
#define PIDDELAY		500

#define RTEMTIMERVALUE  500		//读取温度板数据timer间隔时间

//*****************************************************************
//External variable definition
//*****************************************************************
extern CString sSave;				// save parameter buffer
extern int OperDlgFlag;				// operation dialog message flag
extern int OperReadstautsFlag;		// read status flag
extern BYTE OperBuf [200];			// operation dialog buffer
extern int TemperCycNum;			// 温度循环次数
extern int ReceiveTemCycNum;		// 温度循环接收到数据的次数
//extern int PCRMask;					//被选择PCR标志
extern BOOL AutoSampleFlag;			// opeDlg atuo_sample start flag
extern BOOL temReadTimerFlag;				// PI、Peltier温度循环读取timer,发送指令允许标志

// COperationDlg dialog

class COperationDlg : public CDialog
{
	DECLARE_DYNAMIC(COperationDlg)

public:
	COperationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COperationDlg();

// Dialog Data
	enum { IDD = IDD_OPERATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	CManuDlg m_ManuDlg;
	CParsaveDlg m_ParsaveDlg;
	afx_msg LRESULT OnOpearProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearPIDProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearLEDProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearOvershootProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearCheckPollingProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearSaveRamDataProcess(WPARAM wParam, LPARAM lParam);
	void OperCalMainMsg();
	afx_msg void OnBnClickedBtnOpeLoad();
	afx_msg void OnBnClickedBtnOpeSavestatus();
	afx_msg void OnBnClickedBtnOpeClear();
//	afx_msg void OnEnChangeEditOpeDentem();
	virtual BOOL OnInitDialog();	
	afx_msg void OnEnKillfocusEditOpePitem();
	afx_msg void OnEnKillfocusEditOpeDentem();
	afx_msg void OnEnKillfocusEditOpeDentime();
	afx_msg void OnEnKillfocusEditOpeDeningtem();
	afx_msg void OnEnKillfocusEditOpeDeningtime();
	afx_msg void OnEnKillfocusEditOpeAnntem();
	afx_msg void OnEnKillfocusEditOpeAnntime();
	afx_msg void OnEnKillfocusEditOpeAnncycle();
	afx_msg void OnEnKillfocusEditOpeExttem();
	afx_msg void OnEnKillfocusEditOpeExttime();
	afx_msg void OnEnKillfocusEditOpeHoldtem();
	afx_msg void OnEnKillfocusEditOpeHoldtime();
	afx_msg void OnEnKillfocusEditOpeKp();
	afx_msg void OnEnKillfocusEditOpeKi();
	afx_msg void OnEnKillfocusEditOpeKd();
	afx_msg void OnEnKillfocusEditOpeKl();
	afx_msg void OnBnClickedBtnOpeSavepara();
	afx_msg void OnBnClickedBtnOpeMore();
	void OperReadStatus();
	CString m_operdlg_sEditpel;
	CString m_operdlg_sEditpiread;
	CString m_operdlg_sEditcycleread;
//	CEdit m_operdlg_sEditpwm;
//	CString m_operdlg_sEditpwm;
	CString m_operdlg_sEditvanstatus;
	CString m_operdlg_sEditcurstatus;
	afx_msg void OnBnClickedBtnOpeReadstart();
//	CString m_operdlg_sEditpwm2;
//	CString m_operdlg_sEditpwm1;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnOpeStart();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnOpeLedset();
	afx_msg void OnBnClickedBtnOpeLedread();
	afx_msg void OnBnClickedBtnOpeFanon();
	afx_msg void OnBnClickedBtnOpeFanoff();
	afx_msg void OnBnClickedBtnOpeLedon();
	afx_msg void OnBnClickedBtnOpeLenoff();
	afx_msg void OnBnClickedBtnOpeKpset();
	afx_msg void OnBnClickedBtnOpeKdset();
	afx_msg void OnBnClickedBtnOpeKiset();
	afx_msg void OnBnClickedBtnOpeKlset();
	afx_msg void OnBnClickedBtnOpePidread();
	afx_msg void OnBnClickedBtnOpePidzoneset();
	afx_msg void OnBnClickedBtnOpeStop();
	afx_msg void OnBnClickedBtnOpeFanclosetemp();
	afx_msg void OnBnClickedBtnOpeFanread();
	afx_msg void OnBnClickedBtnOpeOversend();
	afx_msg void OnBnClickedBtnOpeOverread();
	CButton m_opeDlg_ReadStart;
	int m_operdlg_iCheckMask1;
//	BOOL m__operdlg_iCheckMask2;
	int m__operdlg_iCheckMask2;
	int m__operdlg_iCheckMask3;
	int m__operdlg_iCheckMask4;
	//void OperGetPCRMaskStatus();
	afx_msg void OnBnClickedBtnOpeLed2on();
	afx_msg void OnBnClickedBtnOpeLed2off();
	afx_msg void OnBnClickedBtnOpeLed3on();
	afx_msg void OnBnClickedBtnOpeLed3off();
	afx_msg void OnBnClickedBtnOpeLed4on();
	afx_msg void OnBnClickedBtnOpeLed4off();
	afx_msg void OnBnClickedBtnOpeTeSavedata();
	void MakeGraPacket(byte pCmd, byte pType, byte pData[], int num_data);
	CString m_opedlg_sEditpwm1;
	CString m_opedlg_sEditpwm2;
	afx_msg void OnBnClickedButtonSavjson();
	afx_msg void OnBnClickedButtonLdjson();
	void ParseJsonFromFile(CString filename);

	void SetPCRMask(int);
	void SetPiTemp(float);

	void SetPCRCyclTempTime(float den_temp, int den_time, float ann_temp, int ann_time, float ext_temp, int ext_time);
	void SetPCRCyclTempTime2(float iden_temp, int iden_time, float hold_temp, int hold_time, int num_cycle);
	void SetPCRCyclTempTime2Seg(float den_temp, int den_time, float ann_temp, int ann_time);

	void SetPCRCyclTempTimeNew(float den_temp, int den_time, float den_rate, float ann_temp, int ann_time, float ann_rate, float ext_temp, int ext_time, float ext_rate);
	void SetPCRCyclTempTimeNew2Seg(float den_temp, int den_time, float den_rate, float ann_temp, int ann_time, float ann_rate);
	void SetPCRCyclTempTime4Seg(float den_temp, int den_time, float den_ramp, float ann_temp, int ann_time, float ann_ramp, float ann2_temp, int ann2_time, float ann2_ramp, float ext_temp, int ext_time, float ext_ramp);
	void SetPCRCyclTempTime4Seg1303(float den_temp, int den_time, float ann_temp, int ann_time, float ann2_temp, int ann2_time, float ext_temp, int ext_time);

	void ResetParam();
	afx_msg void OnEnKillfocusEditOpeAnntem2();
	afx_msg void OnEnKillfocusEditOpeAnntime2();
	afx_msg void OnEnKillfocusEditOpeDeningramp();
	afx_msg void OnEnKillfocusEditOpeAnnramp();
	afx_msg void OnEnKillfocusEditOpeAnnramp2();
	afx_msg void OnEnKillfocusEditOpeExtramp();
	BOOL m_EnableAnnealing2;
	afx_msg void OnBnClickedCheckAnn2();
	// // Extension phase enable
	BOOL m_EnableExtension;
	afx_msg void OnBnClickedCheckEnext();
	BOOL m_bMaxRamp;
	afx_msg void OnBnClickedCheckMaxramp();

	CChartCtrl m_TChart;
	CChartLineSerie *m_pChartLines[2];		//chart line
	CChartAxis *pAxisBottom;
	CChartAxis *pAxisLeft;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();

	void UpdateSimTempCurve();
	void InitSimTempChart();

	void SetMeltCurve(float start, float end);

	void Loadjson(CString fn, BOOL newp);

	void Savejson(CString fn, BOOL traces);
	afx_msg void OnKillfocusEditOpeOvertime();
	afx_msg void OnKillfocusEditOpeOvertemp();
	afx_msg void OnKillfocusEditOpeOverreadtime();
	afx_msg void OnKillfocusEditOpeOverreadtemp();

	BOOL m_bOvEdited;
	afx_msg void OnEnKillfocusEditOpeMeltstart();
	afx_msg void OnEnKillfocusEditOpeMeltend();
};
