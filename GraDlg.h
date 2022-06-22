#pragma once

#include<vector>


//***************************************************************
//Constant definition
//***************************************************************
#define UM_GRAPROCESS WM_USER+201				//Graphic dialog自定义消息序号
#define UM_GRAPOLLINGPROCESS WM_USER+202		//Graphic dialoa处理polling消息序号
#define UM_GRACYCPOLLPROC	WM_USER + 203		// Zhimin added

#define SENDGRAMSG		1		// graphic dialog向HID发送数据并读取HID command
#define READGRAMSG      2		// 从HID只读取数据 command
#define SENDPAGEMSG		3		// 画页命令
#define SENDVIDEOMSG	4		// video命令
#define STARTTEMHID		5		// 向operDlg发送消息命令
#define GETPCRMASK		6		// 查询operDlg PCR mask状态
#define READSTATUS		7		// Zhimin added, for cycler status polling
#define STARTTHERMCYC	8
#define STOPTHERMCYC	9
#define GRADATANUM 200		   // graphic dialog transmitted data number

#define RowNum24 24		// display row number
#define ColNum24 24		// display column number
//#define pixelsize24	4	// display size for one pixel
#define RowNum12 12		// display row number
#define ColNum12 12		// display column number
//#define pixelsize12 8	// display size for one pixel
//#define arraysize 192	// 8 X 2 X 12

#define dprow12 0x01		// display one line with 12 pixel
#define dppage12 0x02		// display one page with 12 pixel
#define dpvideo12 0x03		// display video with 12 pixel
#define spiread	0x04		// only SPI read
#define adccvt	0x05		// only ADC conversion and SPI read
#define ndread	0x06		// non-destructive read
#define dprow24	0x07		// display one line with 24 pixel
#define dppage24 0x08		// display one page with 24 pixel
#define heatstr 0x09		// start auto-heating
#define heatstop 0x0a		// stop auto-heating
#define dpvideo24 0x0b		// display video with 24 pixel

#define EditClearNum	192		// 显示数据编辑框变量清零行数

#define DRAWPAGE		1		// OnPaint中画页
#define DRAWROW			2		// OnPaint中画行

#define PGRATIMERVALUE  800		//Polling 图像板timer间隔时间

#define MAX_CHAN		4
#define MAX_WELL		16

//*****************************************************************
//External variable
//*****************************************************************
extern byte PCRType;				// PCR select type
extern int PCRCycCnt;				// PCR 依次发送计数
extern int PCRNum;					// 各PCR序号

extern BYTE PCRTypeFilterClass;		// Graphic command返回type 类别区分，如画行、页、vedio（返回type byte的低4位）
extern BYTE PCRTypeFilterNum;		// Graphic command返回type 各PCR区分（返回type byte的高4位）

extern int Page12_24Flag;			// 12行/24行画页标志
extern int Vedio12_24Flag;			// 12行/24行Vedio标志

extern BOOL PollingGraTimerFlag;		// GraDlg polling timer发送指令允许标志
extern BOOL PollingGraMode;				// 画图时判断是否为polling模式


//*****************************************************************
//External function
//*****************************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);		//ASIC convert to HEX
extern int ChangeNum (CString str, int length);								//字符串转10进制

// CGraDlg dialog

class CGraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGraDlg)

public:
	CGraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraDlg();

// Dialog Data
	enum { IDD = IDD_GRAPHIC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_EditReadRow;

	afx_msg LRESULT OnGraProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGraPollingProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGraCycPollProcess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickedBtnReadrow();
	void GraCalMainMsg();
	afx_msg void OnBnClickedBtnDprow24();
	afx_msg void OnBnClickedBtnDppage12();
	afx_msg void OnBnClickedBtnDppage24();
	afx_msg void OnBnClickedBtnDpvedio();
	afx_msg void OnBnClickedBtnStopvideo();
	CButton m_ShowAllData;
	int m_ShowAllDataInt;
	CButton m_ReadHex;
	int m_ReadHexInt;
	CButton m_ReadDec;
	CButton m_ADCData;
	CButton m_ShowValidData;
	CString m_PixelData;
	CString m_Pelt, m_Lidt;
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnAdcconvert();
	CString m_ADCRecdata;
	afx_msg void OnBnClickedBtnClearadc();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedBtnDpvideo24();
	afx_msg void OnBnClickedRadioAdcdata();
	void SetGainMode(int gain);
	void DisplayHDR(void);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void DisplayHDR24(void);
//	int m_GainMode;
//	afx_msg void OnClickedRadiolowgain();
//	afx_msg void OnRadiohighgain();
//	afx_msg void OnRadiohdr();
//	int m_FrameSize;
//	afx_msg void OnClickedRadio12();
//	afx_msg void OnRadio24();
	afx_msg void OnBnClickedButtonCapture();
	void CaptureFrame(BOOL commit);
	void CaptureFrame2(void);
	// void CaptureFrame24(void);
	BOOL m_PixOut;
	afx_msg void OnClickedCheckPixout();
	BOOL m_OrigOut;
	afx_msg void OnClickedCheckOrigout();
	void GraDlgDrawPattern();
	void MakeGraPacket(byte pCmd, byte pType, byte pData);
//	void DrawRaw(CDC *pBufDC, CRect pRect,CBrush pBrush[RowNum24][ColNum24], int pRowNum, int pColNum, int pPixelSize, int pPCRNum);
//	void DrawPage(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum);	
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	void cyclePages(void);
	// void cyclePage24(void);
	void cycleRow12(void);
	void cycleRow24(void);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ReceiveDataProcess(void);
	afx_msg void OnClickedGradlgCheckAutosnap();
	afx_msg void OnBnClickedLedIndvEn();
	afx_msg void OnBnClickedLedSwitch();
	afx_msg void OnBnClickedBtnCopy();
	void UpdatePCRCurve(int, int);
	afx_msg void OnBnClickedPcrcurve();
//	void regDispatch();
	afx_msg void OnBnClickedBtnStarttherm();
	void CoolingDown();
	afx_msg void OnBnClickedBtnClrtmprec();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedButton11();

	void DisplayPollStatus(CString status);
	void DisplayCyclerState(int state);
	void DisplayReceiveTemCycNum(int num);

	void DisplayReportStatus(CString status);

//  dynamic int time

	void DynamicUpdateIntTime();
	BOOL m_dynIntTime[MAX_CHAN]; 
	float m_factorIntTime[MAX_CHAN];
	int  m_maxPixVal[MAX_CHAN];

	CString m_Poll, m_lastStr, m_Report;
	BOOL m_DelayTrigger;
	afx_msg void OnBnClickedCheckDelaytrigger();
	BOOL m_Melt;
	afx_msg void OnBnClickedCheckMelt();

	void OnAutocalibInt();
	int AutocalibInt();
	int GetMaxChanRead(int ch);
	BOOL m_ShowMarker;
	afx_msg void OnBnClickedCheckShowmarker();
	BOOL m_bTestLid;
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedButtonMarkerup();

	void RefreshMarkers();
	afx_msg void OnBnClickedButtonMarkerleft();
	afx_msg void OnBnClickedButtonMarkerdown();
	afx_msg void OnBnClickedButtonMarkerright();
	afx_msg void OnBnClickedButton14();

	void ValidateTempCtrl();
	afx_msg void OnBnClickedBtnCopy2();
};
