// GraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "GraDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <afx.h>
#include <afxres.h>
#include "CurveShow.h"
#include <vector>
#include <regex>
#include "SystemConfig.cpp"

//added by lixin 20190327
#include "MathRound.cpp"

using namespace std;

#include "OperationDlg.h"

//***************************************************************
//Global variable definition
//***************************************************************

int GraFlag = 0;				// graphic dialog flag
bool Gra_pageFlag = false;		// graphic dialog »­Ò³Ñ­»·±êÖ¾
bool Gra_videoFlag = false;		// graphic dialog videoÑ­»·±êÖ¾
BYTE GraBuf[GRADATANUM];		// graphic dialog transmitted data buffer
int RecdataFlag = 0;			// receive data recode flag

byte PCRType = 0;				// PCR select type
int PCRCycCnt = 0;				// PCR ÒÀ´Î·¢ËÍ¼ÆÊý
int PCRNum = 0;					// ¸÷PCRÐòºÅ

BYTE PCRTypeFilterClass = 0;	// Graphic command·µ»Øtype Àà±ðÇø·Ö£¬Èç»­ÐÐ¡¢Ò³¡¢vedio£¨·µ»Øtype byteµÄµÍ4Î»£©
BYTE PCRTypeFilterNum = 0;		// Graphic command·µ»Øtype ¸÷PCRÇø·Ö£¨·µ»Øtype byteµÄ¸ß4Î»£©

int Page12_24Flag = 0;			// 12ÐÐ/24ÐÐ»­Ò³±êÖ¾(0:12ÐÐ£¬1:24ÐÐ)
int Vedio12_24Flag = 0;			// 12ÐÐ/24ÐÐVedio±êÖ¾(0:12ÐÐ£¬1:24ÐÐ)

int xOffset = 0;				// ¸÷PCR»­Í¼Æ«ÒÆÁ¿
int yOffset = 0;				// ¸÷PCR»­Í¼Æ«ÒÆÁ¿

CBitmap bBmp;						// ºóÌ¨»­Í¼BMP
CBitmap *oldBMP;					// Ô­BMPÖ¸Õë
CDC bgDC;							// ºóÌ¨DC
CDC * dispalyDC;					// Ç°Ì¨ÏÔÊ¾DC
CRect displayRect;					// ÏÔÊ¾ÓÃrect

CBitmap bBmp_Row;					// ºóÌ¨»­ÐÐBMP
CBitmap *oldBMP_Row;				// Ô­BMPÖ¸Õë
CDC bgDC_Row;						// ºóÌ¨DC
CDC *RowDC;							// Ç°Ì¨ÏÔÊ¾DC

int OnPaint_Flag = 0;				// OnPaint»æÖÆ±êÖ¾
BOOL DrawPageStart = FALSE;			// ÅÐ¶ÏÊÇ²»ÊÇµÚÒ»´Î°´»æÍ¼°´Å¥(Capture)
BOOL DrawRowStart = FALSE;			// ÅÐ¶ÏÊÇ²»ÊÇµÚÒ»´Î°´»­ÐÐ°´Å¥

CString sPCRdata;
CString sADCRecdata;

//CFile m_recdataFile(_T("PRCdata.txt"),CFile::modeCreate|CFile::modeWrite);			// receive data file
//CFile m_adcdataFile(_T("ADCdata.txt"),CFile::modeCreate|CFile::modeWrite);			// ADC data file

CFile m_recdataFile;			// receive data file
CFile m_adcdataFile;			// ADC data file

CTime sysTime_Gra;					// system time

POINT pt;			// ±à¼­¿òÏÔÊ¾Î»ÖÃÖ¸Õë
int typePixelSize = 0;		// 12ÐÐºÍ24ÐÐÇø·Ö±êÖ¾

BOOL PollingGraTimerFlag = FALSE;		// GraDlg polling timer·¢ËÍÖ¸ÁîÔÊÐí±êÖ¾
BOOL PollingGraMode = FALSE;			// »­Í¼Ê±ÅÐ¶ÏÊÇ·ñÎªpollingÄ£Ê½

BOOL PollingPCRMask = TRUE;				// ÅÐ¶ÏËùÑ¡PCRÊ±polling±êÖ¾ 

int pixelNum = 12;						// Default 12X12 frame

int pixelsize24 = 4;	// display size for one pixel
int pixelsize12 = 8;	// display size for one pixel
int arraysize = 192;	// 8 X 2 X 12

std::vector<double> m_yData[MAX_CHAN][MAX_WELL];	//handled data£¬4 channels£¬4 frames // Zhimin comment: first index is wavelength channels, second index is wells.

// std::vector<double> m_factorInt[MAX_CHAN];

std::vector<std::string> m_PositionChip[MAX_CHAN];

int numWells = MAX_WELL;
int numChannels = MAX_CHAN;
int wellFormat = 0;				// non-specified value

extern std::vector <double> ifactor[MAX_CHAN];
extern std::vector <double> mtemp[MAX_CHAN];

std::vector <int> v_frame_data[4][24];

extern ReportStrings test_report;

CTime cycleStart;


//*****************************************************************
//Own function
//*****************************************************************


//*****************************************************************
//External function
//*****************************************************************


extern float int_time; // zd add, in ms
extern int gain_mode;  // zd add, 0 high gain, 1 low gain
extern int frame_size;
int adc_result[24];    // zd add
int frame_data[24][24];		// A whole frame of data
int rn;
float melt_data[4];

extern BOOL g_DeviceDetected;

//=====HDR Support==========

int pdata_0[24][24];	// first pass
int pdata_1[24][24];	// second pass
int hdr_phase = 0;

int contrast = 16;

extern CTrimDlg *g_pTrimDlg;

extern CTrimReader *g_pTrimReader;
extern DPReader *g_pDPReader;

#define DARK_LEVEL 100					// numerical dark level

#define ADC_CORRECT
#define DARK_MANAGE

#define SAVE_DATA_FILE
#define SAVE_REC_FILE

CCurveShow *pCVS = NULL;		// Zhimin added. This is for modeless implementation of the curve show dialog.

CString ADCfilename;				//adc file name

BOOL ping_pong = 0;

float g_LidTemp = 40;

int cyc_status = 0; // 0 stopped, 1 running, 2 cooling down

long outlier_pixel[MAX_CHAN];

//------------------

// CGraDlg dialog

IMPLEMENT_DYNAMIC(CGraDlg, CDialogEx)

CGraDlg::CGraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraDlg::IDD, pParent)
//	, m_GainMode(0)				// Low gain default
//	, m_FrameSize(0)
	, m_OrigOut(FALSE)
	, m_DelayTrigger(FALSE)
	, m_Melt(FALSE)
	, m_ShowMarker(FALSE)
	, m_bTestLid(FALSE)
{

	m_EditReadRow = _T("");
	m_ShowAllDataInt = 0;
	m_ReadHexInt = 0;
	m_PixelData = _T("");
	m_ADCRecdata = _T("");

	m_Pelt = _T("");
	m_Lidt = _T("");
	m_Poll = _T("");
	m_lastStr = _T("");
	m_Report = _T("");

//	m_GainMode = 1;
//	m_FrameSize = 0;
	m_PixOut = true;
	m_OrigOut = false;

	for(int i=0; i<24; i++) {
		adc_result[i] = 0;
	}

	for (int i = 0; i < 4; i++) {
		m_PositionChip[i].empty();
	}

	for (int i = 0; i < MAX_CHAN; i++) {
		m_dynIntTime[i] = false;
		m_factorIntTime[i] = 1.0;
		m_maxPixVal[i] = DARK_LEVEL;
	}

	for (int i = 0; i < MAX_CHAN; i++) {
		for (int j = 0; j < 24; j++) {
			v_frame_data[i][j].resize(24);
		}
		outlier_pixel[i] = 0;
	}
}

CGraDlg::~CGraDlg()
{
}

void CGraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_READROW, m_EditReadRow);
	DDX_Control(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllData);
	DDX_Radio(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllDataInt);
	DDX_Control(pDX, IDC_RADIO_READHEX, m_ReadHex);
	DDX_Radio(pDX, IDC_RADIO_READHEX, m_ReadHexInt);
	DDX_Control(pDX, IDC_RADIO_READDEC, m_ReadDec);
	DDX_Control(pDX, IDC_RADIO_ADCDATA, m_ADCData);
	DDX_Control(pDX, IDC_RADIO_SHOWVALIDDATA, m_ShowValidData);
	DDX_Text(pDX, IDC_EDIT_RecData, m_PixelData);
	DDX_Text(pDX, IDC_EDIT_ADCDATA, m_ADCRecdata);
	//  DDX_Radio(pDX, IDC_RADIOLOWGAIN, m_GainMode);
	//  DDX_Radio(pDX, IDC_RADIO_12, m_FrameSize);
	DDX_Check(pDX, IDC_CHECK_PIXOUT, m_PixOut);
	DDX_Check(pDX, IDC_CHECK_ORIGOUT, m_OrigOut);
	//	DDX_Check(pDX, IDC_CHECK_DELAYTRIGGER, m_DelayTrigger);
	DDX_Check(pDX, IDC_CHECK_MELT, m_Melt);
	DDX_Check(pDX, IDC_CHECK_SHOWMARKER, m_ShowMarker);
	DDX_Check(pDX, IDC_CHECK6, m_bTestLid);
}


BEGIN_MESSAGE_MAP(CGraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_READROW, &CGraDlg::OnClickedBtnReadrow)
	ON_MESSAGE(UM_GRAPROCESS,OnGraProcess)
	ON_MESSAGE(UM_GRACYCPOLLPROC, OnGraCycPollProcess)
	ON_MESSAGE(UM_GRAPOLLINGPROCESS,OnGraPollingProcess)
	ON_BN_CLICKED(IDC_BTN_DPROW24, &CGraDlg::OnBnClickedBtnDprow24)
	ON_BN_CLICKED(IDC_BTN_DPPAGE12, &CGraDlg::OnBnClickedBtnDppage12)
	ON_BN_CLICKED(IDC_BTN_DPPAGE24, &CGraDlg::OnBnClickedBtnDppage24)
	ON_BN_CLICKED(IDC_BTN_DPVEDIO, &CGraDlg::OnBnClickedBtnDpvedio)
	ON_BN_CLICKED(IDC_BTN_STOPVIDEO, &CGraDlg::OnBnClickedBtnStopvideo)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CGraDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_ADCCONVERT, &CGraDlg::OnBnClickedBtnAdcconvert)
	ON_BN_CLICKED(IDC_BTN_CLEARADC, &CGraDlg::OnBnClickedBtnClearadc)
	ON_BN_CLICKED(IDC_BUTTON1, &CGraDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CGraDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_DPVIDEO24, &CGraDlg::OnBnClickedBtnDpvideo24)
	ON_BN_CLICKED(IDC_RADIO_ADCDATA, &CGraDlg::OnBnClickedRadioAdcdata)
	ON_WM_HSCROLL()
//	ON_BN_CLICKED(IDC_RADIOLOWGAIN, &CGraDlg::OnClickedRadiolowgain)
//	ON_COMMAND(IDC_RADIOHIGHGAIN, &CGraDlg::OnRadiohighgain)
//	ON_COMMAND(IDC_RADIOHDR, &CGraDlg::OnRadiohdr)
//	ON_BN_CLICKED(IDC_RADIO_12, &CGraDlg::OnClickedRadio12)
//	ON_COMMAND(IDC_RADIO_24, &CGraDlg::OnRadio24)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CGraDlg::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_CHECK_PIXOUT, &CGraDlg::OnClickedCheckPixout)
	ON_BN_CLICKED(IDC_CHECK_ORIGOUT, &CGraDlg::OnClickedCheckOrigout)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_GraDlg_Check_AutoSnap, &CGraDlg::OnClickedGradlgCheckAutosnap)
	ON_BN_CLICKED(IDC_LED_INDV_EN, &CGraDlg::OnBnClickedLedIndvEn)
	ON_BN_CLICKED(IDC_LED_SWITCH, &CGraDlg::OnBnClickedLedSwitch)
	ON_BN_CLICKED(IDC_BTN_COPY, &CGraDlg::OnBnClickedBtnCopy)
	ON_BN_CLICKED(IDC_PCRCURVE, &CGraDlg::OnBnClickedPcrcurve)
	ON_BN_CLICKED(IDC_BTN_STARTTHERM, &CGraDlg::OnBnClickedBtnStarttherm)
	ON_BN_CLICKED(IDC_BTN_CLRTMPREC, &CGraDlg::OnBnClickedBtnClrtmprec)
	ON_BN_CLICKED(IDC_BUTTON9, &CGraDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON8, &CGraDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON10, &CGraDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &CGraDlg::OnBnClickedButton11)
//	ON_BN_CLICKED(IDC_CHECK_DELAYTRIGGER, &CGraDlg::OnBnClickedCheckDelaytrigger)
ON_BN_CLICKED(IDC_CHECK_MELT, &CGraDlg::OnBnClickedCheckMelt)
ON_BN_CLICKED(IDC_CHECK_SHOWMARKER, &CGraDlg::OnBnClickedCheckShowmarker)
ON_BN_CLICKED(IDC_CHECK6, &CGraDlg::OnBnClickedCheck6)
ON_BN_CLICKED(IDC_BUTTON_MARKERUP, &CGraDlg::OnBnClickedButtonMarkerup)
ON_BN_CLICKED(IDC_BUTTON_MARKERLEFT, &CGraDlg::OnBnClickedButtonMarkerleft)
ON_BN_CLICKED(IDC_BUTTON_MARKERDOWN, &CGraDlg::OnBnClickedButtonMarkerdown)
ON_BN_CLICKED(IDC_BUTTON_MARKERRIGHT, &CGraDlg::OnBnClickedButtonMarkerright)
ON_BN_CLICKED(IDC_BUTTON14, &CGraDlg::OnBnClickedButton14)
ON_BN_CLICKED(IDC_BTN_COPY2, &CGraDlg::OnBnClickedBtnCopy2)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//µ÷ÓÃÖ÷¶Ô»°¿ò¶ÔÓ¦ÏûÏ¢´¦Àíº¯Êý
void CGraDlg::GraCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_GraDlg_event,a,b);
}


void CGraDlg::MakeGraPacket(byte pCmd, byte pType, byte pData)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = pCmd;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = pType;		//data type, date edit first byte
	TxData[4] = pData;		//real data
	TxData[5] = 0x00;		//Ô¤ÁôÎ»
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code
}

/*
void CGraDlg::DrawRaw(CDC *pBufDC, CRect pRect,CBrush pBrush[RowNum24][ColNum24], int pRowNum, int pColNum, int pPixelSize, int pPCRNum)
{
	int xOffset = 0;
	int yOffset = 0;
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = 72;
			yOffset = 0;
		}
		break;
	case 3:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 0;
			yOffset = 60;
		}
		else
		{
			xOffset = 0;
			yOffset = 72;
		}
		break;
	case 4:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;
			yOffset = 60;
		}
		else
		{
			xOffset = 72;
			yOffset = 72;
		}
		break;
	default:
		break;
	}

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pBufDC,pRowNum*pPixelSize,pColNum*pPixelSize);		// ÉèÖÃµ×Ä»´óÐ¡

	CDC bDC; 
	bDC.CreateCompatibleDC(pBufDC);	

	bDC.SelectObject(&bmp);		

	for(int l=0; l<pColNum; l++)		// l´ú±íÁÐºÅ£»rxdata[5]ÖÐÊý¾ÝÊÇÐÐºÅ
	{
		pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
		bDC.Rectangle(pRect);
		bDC.FillRect(&pRect,&pBrush[RxData[5]][l]);
	}
	pBufDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);
}

void CGraDlg::DrawPage(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum)
{
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
		{
			xOffset = pixelsize12*12;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = 0;
		}
		break;
	case 3:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 0;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = 0;
			yOffset = pixelsize24*24;
		}
		break;
	case 4:
		if (pPixelSize == pixelsize12)
		{
			xOffset = pixelsize12*12;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = pixelsize24*24;
		}
		break;
	default:
		break;
	}

	if (pPixelSize == pixelsize12)
	{
		for(int l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
		{
			if(gain_mode <= 1) {
				pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));

				pBufDC->Rectangle(pRect);
				pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
			}
			else {

				//zd add

				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
			}
		}
	}
	else
	{
		for(int l=0; l<ColNum24; l++)		// l´ú±íÁÐºÅ
		{
			if(gain_mode <= 1) {
				pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
				pBufDC->Rectangle(pRect);
				pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
			}
			else {

				//zd add

				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
			}
		}
	}

}

void DrawOverTime(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum)
{
	int xOffset = 0;
	int yOffset = 0;
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = 72;
			yOffset = 0;
		}
		break;
	case 3:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 0;
			yOffset = 60;
		}
		else
		{
			xOffset = 0;
			yOffset = 72;
		}
		break;
	case 4:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;
			yOffset = 60;
		}
		else
		{
			xOffset = 72;
			yOffset = 72;
		}
		break;
	default:
		break;
	}

	for(int l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
	{
		pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
		pBufDC->Rectangle(pRect);
		pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
	}
}
*/
void CGraDlg::ReceiveDataProcess()
{
	//Êý¾Ý´¦Àí¼°ÏÔÊ¾

	// ¸ù¾Ý·µ»ØµÄtypeÇø·ÖÊÇÏÔÊ¾12ÁÐÊý¾Ý»¹ÊÇ24ÁÐÊý¾Ý
	switch(PCRTypeFilterClass)
	{
	case dprow12:		// 12ÁÐ»­ÐÐ
	case dppage12:		// 12ÁÐ»­Ò³
	case dpvideo12:		// 12ÁÐ»­vedio
	case 0x0c:			// »ý·ÖÊ±¼ä³¬¹ý10ms
		pixelNum = 12;
		break;

	case dprow24:		// 24ÁÐ»­ÐÐ
	case dppage24:		// 24ÁÐ»­Ò³
	case dpvideo24:		// 24ÁÐ»­vedio
		pixelNum = 24;
		break;

	default: break;
	}

	WORD * DecData = new WORD [pixelNum];
	int NumData = 0;
	int iDecVal = 0;
	CString TmHex;
	TmHex.Empty();
	CString sDecData;	// ×îºóÏÔÊ¾µÄÊ®½øÖÆ×Ö·û´®
	sDecData.Empty();
	char sDecVal[10];
	BYTE lByte = 0x00;
	CString sRownum;
	sRownum.Empty();
	sRownum.Format(" %d",RxData[5]);

	for (NumData=0; NumData<pixelNum; NumData++)			//½«Ã¿Á½¸öbyteÕûºÏ³ÉÒ»¸öword
	{
		lByte=RxData[NumData*2+6];				//È¡³öµÍ4Î»byte
		lByte = lByte<<4;						//½«µÍ4Î»byte×óÒÆ4Î»
		DecData[NumData] = RxData[NumData*2+7];		//½«¸ß8Î»byte¸³Öµ¸øword±äÁ¿
		DecData[NumData] <<= 8;						//word buffer×óÒÆ8Î»£¬½«¸ß8Î»byteÊý¾Ý·ÅÖÃµ½¸ß8Î»
		DecData[NumData] |= lByte;				//½«µÍ4Î»byte·Åµ½word bufferµÍ8Î»
		DecData[NumData] >>= 4;						//½«word bufferÕûÌåÓÒÒÆ4Î»£¬±ä³ÉÓÐÐ§12Î»Êý¾Ý
	}

	for (NumData=0; NumData<pixelNum; NumData++)			//½«Ò»ÐÐÖÐµÄÃ¿¸öPixelÊ®Áù½øÖÆÓÐÐ§Êý¾Ý×ª³ÉÊ®½øÖÆ
	{
		TmHex.Format("%2X",DecData[NumData]);		//½«Ã¿¸öword buffer×ª³ÉÊ®Áù½øÖÆ×Ö·û´®
		iDecVal = ChangeNum(TmHex, TmHex.GetLength());		//½«Ã¿¸öÊ®Áù½øÖÆ×Ö·û´®×ª³ÉÓÐÐ§Ê®½øÖÆÊý
		gcvt(iDecVal,4,sDecVal);		//½«Ê®½øÖÆ¸¡µãÊý×ª»»³É×Ö·û´®
		//µÚ¶þ¸ö²ÎÊý´ú±íÊ®½øÖÆÊýÓÐ¶àÉÙÎ»
		//µÚÈý¸ö²ÎÊý£¬±ØÐëÎªchar *, ÈçÉÏÃæ¶ÔsDecValµÄ¶¨Òå

		sDecData += sDecVal;				//½«char* ¸³Öµ¸øCString£¬ÓÃÀ´ÏÔÊ¾
		sDecData += "  ";					//Ã¿¸öÊý¾Ý¼ä¼Ó¿Õ¸ñ
	}

	sDecData += sRownum;
	delete[] DecData;

	// µ±·µ»ØÃüÁîÎª0x14Ê±£¬Ö»ÏÔÊ¾ÏÂÊö×Ö·û´®
	if (RxData[2] == 0x14)
	{
		sDecData = "";
		sDecData.Format("the cycle number is %d",RxData[3]);
	}

	int result = 0;
	char fstrbuf[9];

	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û´®
	sADCData.Empty();

	int flag;		// For ADC Correct

	if(RxData[5] == 0) {
		if(PCRNum == 1) 
			sADCData += "\r\n";
		
		sADCData += "Chip#";
		itoa (PCRNum, fstrbuf, 9);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
		sADCData += fstrbuf;
		sADCData += "\r\n";
	}

	for (NumData=0; NumData<pixelNum; NumData++)
	{
		if(g_pTrimReader) {		
			result = g_pTrimReader->ADCCorrection(NumData, RxData[NumData*2+7], RxData[NumData*2+6], pixelNum, PCRNum, gain_mode, &flag);
		}
		else {
			result = 0;
		}

		int row = RxData[5];
		int col = NumData;

		if (row == 11 && col == 11 && pixelNum == 12) {			// special code, use position [11][11] to encode int factor
			result = 5000 + (int)(m_factorIntTime[PCRNum - 1] * 10000);
		}

		adc_result[col] = result; // zd add

		frame_data[row][col] = result;		// RxData is the row number.

		v_frame_data[PCRNum - 1][row].at(col) = result;

		if (result < 50 || result > 150) outlier_pixel[PCRNum - 1] += 1;

		itoa (result,fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
		sADCData += fstrbuf;
		sADCData += " ";
	}

	//==========Get melt temp data=============

	unsigned char cTem[4];
	float * fTem;
	CString str;

	if (m_Melt && RxData[5] == 0 && RxData[3] > 28 && cyc_status == 1) {		// RxData[3] is the packet length
		cTem[0] = RxData[pixelNum * 2 + 6];
		cTem[1] = RxData[pixelNum * 2 + 7];
		cTem[2] = RxData[pixelNum * 2 + 8];
		cTem[3] = RxData[pixelNum * 2 + 9];

		fTem = (float *)cTem;
		melt_data[PCRNum - 1] = *fTem;

		str.Format("Melt temp: %0.2f, chan %d, packlen %d", melt_data[PCRNum - 1], PCRNum - 1, RxData[3]);
		DisplayPollStatus(str);
	}

	//=========================================

	sADCData += sRownum;

	// µ±·µ»ØÃüÁîÎª0x14Ê±£¬Ö»ÏÔÊ¾ÏÂÊö×Ö·û´®
	if (RxData[2] == 0x14)
	{
		sADCData = "";
		sADCData.Format("the cycle number is %d",RxData[3]);
	}

	if (m_OrigOut && m_ReadHex.GetCheck())			//ÒÔÊ®Áù½øÖÆ
	{
		if (m_ShowAllData.GetCheck())	//ÏÔÊ¾ËùÓÐÊý¾Ý
		{
			//			m_PixelData = RegRecStr + m_PixelData;		//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
			m_PixelData += (RegRecStr+"\r\n");					//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (RegRecStr+"\r\n");

			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë			
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			RegRecStr = "";
		}

		if (m_ShowValidData.GetCheck())	//ÏÔÊ¾ÓÐÐ§Êý¾Ý
		{
			//			m_PixelData = Valid_RegRecStr + m_PixelData;	//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
			m_PixelData += (Valid_RegRecStr+"\r\n");					//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (Valid_RegRecStr+"\r\n");

			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			//			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			Valid_RegRecStr = "";
		}		
	}

	if (m_OrigOut && m_ReadDec.GetCheck())			//ÒÔÊ®½øÖÆÏÔÊ¾
	{
		//		m_PixelData = sTest + m_PixelData +"\r\n";			//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿òµÚÒ»ÐÐÏÔÊ¾
		m_PixelData += (sDecData+"\r\n");						
		SetDlgItemText(IDC_EDIT_RecData,m_PixelData);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾	

		sPCRdata = (sDecData+"\r\n");

		//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
		//		POINT pt;
		GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}

	if (m_PixOut && gain_mode <= 1)		//ADCÊý¾ÝÏÔÊ¾
	{
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
		//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾

		sADCRecdata = (sADCData+"\r\n");

		//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
		//		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

	}

	if(RxData[5] == 11 && pixelNum == 12) {	
		UpdatePCRCurve(PCRNum, pixelNum);
	}
	else if(RxData[5] == 23 && pixelNum == 24) {
		UpdatePCRCurve(PCRNum, pixelNum);
	}
} 


void CGraDlg::UpdatePCRCurve(int PCRNum, int pixelNum)
{
	// This is called when a frame of image data is captured. 
	// PCRNum is the chip number. It is 1 based, i.e. 1,2,3,4.
	// pixelNum is the frame size, 12 for 12X12 frames and 24 for 24x24 frames
	// The frame data is available in the global variable int frame_data[24][24].

	switch (PCRNum)
	{
	case 1:
		if (typePixelSize == 12)		// 12ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12 * 12;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = pixelsize24 * 24;
			yOffset = 0;
		}
		break;
	case 3:
		if (typePixelSize == 12)
		{
			xOffset = 0;
			yOffset = pixelsize12 * 12;
		}
		else
		{
			xOffset = 0;
			yOffset = pixelsize24 * 24;
		}
		break;
	case 4:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12 * 12;
			yOffset = pixelsize12 * 12;
		}
		else
		{
			xOffset = pixelsize24 * 24;
			yOffset = pixelsize24 * 24;
		}
		break;
	default:
		break;
	}

	//calculate yData 
	if (pixelNum == 24)
	{
		for (int row = 0; row<24; row = row + 6)			// for every amount from 6 rows and 6 columns to produce 1 singal
		{
			int sum = 0;					// record the frame
			for (int innerrow = row; innerrow<row + 6; innerrow++)	//handle 6 rows 
			{
				for (int column = 6; column<12; column++)
				{   //handle 6 columns in every row, every number subtract 100 to get rid of noice.And plus them.
					int value = frame_data[innerrow][column] - DARK_LEVEL;
					sum += abs(value)<10 ? 0 : value;
				}
			}
			m_yData[PCRNum - 1][row / 6].push_back(sum);
		}
	}
	else if (pixelNum == 12)
	{
		int max = DARK_LEVEL;		// new max

		int last_max = m_maxPixVal[PCRNum - 1];

		if (m_dynIntTime[PCRNum - 1]) {
			last_max *= 0.5;
		}

		for (int frame = 0; frame<numWells; frame++)
		{
/*			int j = 0;
			int column[10], row[10];
			memset(column, 0, 10 * sizeof(int));
			memset(row, 0, 10 * sizeof(int));
			CString temp(m_PositionChip[PCRNum - 1].at(frame).c_str());
			CString number;

			for (size_t i = 0; i<m_PositionChip[PCRNum - 1].at(frame).size(); i++)
			{
				char tt = temp.GetAt(i);
				if (tt >= 'A'&& tt <= 'Z') {
					column[j] = tt - 'A';
				}
				else if (tt == '+') {
					row[j] = atoi(number);
					j++;
					number = "";
				}
				else if (tt >= '0'&& tt <= '10') {
					number += tt;
				}
			}

			row[j] = atoi(number);
			number = "";
*/
			double sum = 0;
			int j = g_pDPReader->row_index[PCRNum - 1][frame].size();
			int row, col;

			for (int i = 0; i < j; i++)
			{
				row = g_pDPReader->row_index[PCRNum - 1][frame].at(i);
				col = g_pDPReader->col_index[PCRNum - 1][frame].at(i);

				sum += frame_data[row][col] - DARK_LEVEL;		// Subtract numerical dark

				if (m_ShowMarker) {

					displayRect.SetRect((pixelsize12 * col) + xOffset, 
						(pixelsize12 * row) + yOffset, 
						(pixelsize12 * (col + 1)) + xOffset, 
						(pixelsize12 * (row + 1)) + yOffset);

					bgDC.DrawEdge(displayRect, EDGE_ETCHED, BF_RECT);
				}

				if (frame_data[row][col] > max) {
					max = frame_data[row][col];
				}
			}

			if (m_yData[PCRNum - 1][frame].empty()) {
				m_yData[PCRNum - 1][frame].push_back(sum);		// First time push twice.
			}
			m_yData[PCRNum - 1][frame].push_back(sum);
		}

		float factor = m_factorIntTime[PCRNum - 1];

		if (ifactor[PCRNum - 1].empty()) {
			ifactor[PCRNum - 1].push_back(factor);
		}
		ifactor[PCRNum - 1].push_back(factor);

		// Decide whether to adjust int time for the next capture

		m_maxPixVal[PCRNum - 1] = max;

		if (max + (max - last_max) > 3300) {
			m_dynIntTime[PCRNum - 1] = true;

			CString str;
			str.Format("Overflow detected max %d, last max %d", max, last_max);
			DisplayPollStatus(str);
		}
		else {
			CString str;
			str.Format("Current pixel max max %d, last max %d", max, last_max);
			DisplayPollStatus(str);
		}
	}
}

void CGraDlg::DynamicUpdateIntTime() 
{
	for (int i = 0; i < MAX_CHAN; i++) {
		
//		if (m_factorInt[i].empty()) {
//			m_factorInt[i].push_back(m_factorIntTime[i]);    // First time push twice
//		}
//		m_factorInt[i].push_back(m_factorIntTime[i]);

		if (m_dynIntTime[i] && m_factorIntTime[i] > 0.03) {
			m_factorIntTime[i] *= 0.5;

			// Call to update Int time
			float new_factor;
			new_factor = g_pTrimDlg->DynamicUpdateIntTime(m_factorIntTime[i], i);	// done here because we need to set int time before auto trigger happens.
			m_factorIntTime[i] = new_factor;

			m_dynIntTime[i] = false;

			CString str;
			str.Format("Chan %d, IntTime factor %3.2f", i + 1, m_factorIntTime[i]);
			DisplayPollStatus(str);
		}
	}
}

LRESULT CGraDlg::OnGraProcess(WPARAM wParam, LPARAM lParam)
{
	//Êý¾Ý´¦Àí
	ReceiveDataProcess();

#ifdef SAVE_DATA_FILE

	m_adcdataFile.SeekToEnd();

	// ADC data save file
	m_adcdataFile.Write(sADCRecdata,sADCRecdata.GetLength());

	// ±£´æÉÏÒ»´Î°´¼üµÄÎÄ¼þ
	if ((PCRMask == 0) & (Gra_pageFlag == false))
	{
		m_adcdataFile.Close();
	}

#endif

#ifdef SAVE_REC_FILE

//	m_recdataFile.SeekToEnd();

	if ((PCRMask == 0) && (Gra_pageFlag == false) && m_recdataFile.m_hFile != CFile::hFileNull)
	{
		m_recdataFile.Write(m_Poll, m_Poll.GetLength());
		m_recdataFile.Close();										// Zhimin: save it for every frame set?
	}

#endif

	// Drawing ´¦Àí
	GraDlgDrawPattern();
/*
	// »­Ò³Ê±Ã¿´Î¶ÁÈ¡Ò»ÐÐ£¬Êý¾Ý´¦ÀíºÃÔÙ¶ÁÏÂÒ»ÐÐ
	if (Gra_pageFlag)
	{
		GraFlag = READGRAMSG;
		GraCalMainMsg();
	}
*/
	return 0;
}


LRESULT CGraDlg::OnGraPollingProcess(WPARAM wParam, LPARAM lParam)
{
	PollingGraMode = TRUE;
	PollingGraTimerFlag = TRUE;

	SetTimer(2,PGRATIMERVALUE,NULL);		// start polling

	DisplayPollStatus("Polling Gra HID...(start timer 2)");

	return 0;
}


void CGraDlg::GraDlgDrawPattern()
{
	CBrush brush[RowNum24][ColNum24];	// ÐÂ»­Ë¢

	int i=0;
	int l=0;
	int gray_level=0;

	// zd comment RxData[5] is the row number.

    rn = RxData[5];

	for (i=0; i<ColNum24; i++)
	{
		gray_level = adc_result[i]/contrast;

		if (adc_result[i] >= 5000) gray_level = 100;

		if(gray_level > 255) gray_level = 255;
		else if(gray_level < 0) gray_level = 0;

		brush[RxData[5]][i].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));				 // zd mod use corrected adc data with green tint
//		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+6],RxData[i*2+6],RxData[i*2+6]));		// È¡¸ßbyteÎªÓÐÐ§Î»
	}

	switch (PCRNum)
	{
	case 1:
		if (typePixelSize == 12)		// 12ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24ÐÐÇé¿ö
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12*12;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = 0;
		}
		break;
	case 3:
		if (typePixelSize == 12)
		{
			xOffset = 0;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = 0;
			yOffset = pixelsize24*24;
		}
		break;
	case 4:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12*12;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = pixelsize24*24;
		}
		break;
	default:
		break;
	}

	switch(PCRTypeFilterClass)
	{
	case dprow12:	
		{
			for(int l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ£»rxdata[5]ÖÐÊý¾ÝÊÇÐÐºÅ
			{
				displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
				bgDC_Row.Rectangle(displayRect);
				bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
			}

			if (PCRMask == 0)	// ËùÑ¡PCRÒÑÈ«²¿»­Íê
			{
				PCRNum = 0;

				OnPaint_Flag = DRAWROW;
				Invalidate();
			}
			else
			{
				//Êý¾Ý´«ÊäbufferÇåÁã
				memset(RxData,0,sizeof(RxData));

				cycleRow12();		// ·¢ËÍÏÂÒ»¸öPCR»­ÐÐÖ¸Áî
			}	

			break;
		}
	case dppage12:	
		{
			for(int l=0; l<ColNum12; l++)		// l´ú±íÁÐºÅ
			{
				if(gain_mode <= 1) 
				{
					displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
					bgDC.Rectangle(displayRect);
					bgDC.FillRect(&displayRect,&brush[RxData[5]][l]);
				}
				else
				{
					//zd add
					if(!hdr_phase) {
						pdata_0[rn][l] = adc_result[l];
					}
					else {
						pdata_1[rn][l] = adc_result[l];
					}
				}
			}

			break;
		}
	case 0x0c:		// »ý·ÖÊ±¼ä³¬¹ý10ms
		{
			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dprow24:	
		{
			for(int l=0; l<ColNum24; l++)		// l´ú±íÁÐºÅ£»rxdata[5]ÖÐÊý¾ÝÊÇÐÐºÅ
			{
				displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
				bgDC_Row.Rectangle(displayRect);
				bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
			}

			if (PCRMask == 0)		// ËùÑ¡PCRÒÑÈ«²¿»­Íê
			{
				PCRNum = 0;

				OnPaint_Flag = DRAWROW;
				Invalidate();
			}
			else
			{
				//Êý¾Ý´«ÊäbufferÇåÁã
				memset(RxData,0,sizeof(RxData));

				cycleRow24();		// ·¢ËÍÏÂÒ»¸öPCR»­ÐÐÖ¸Áî
			}
			break;
		}
	case dppage24:	
		{
			for(int l=0; l<ColNum24; l++)		// l´ú±íÁÐºÅ
			{
				if(gain_mode <= 1) 
				{
					displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
					bgDC.Rectangle(displayRect);
					bgDC.FillRect(&displayRect,&brush[RxData[5]][l]);
				}
				else
				{
					//zd add
					if(!hdr_phase) {
						pdata_0[rn][l] = adc_result[l];
					}
					else {
						pdata_1[rn][l] = adc_result[l];
					}
				}
			}
			break;
		}
	case dpvideo12:	//video12
		{
			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo24:	//video24
		{
			//Êý¾Ý´«ÊäbufferÇåÁã
			memset(RxData,0,sizeof(RxData));

			break;
		}

	default:
		break;
	}

	// release memeory
	for (int i=0; i<RowNum24; i++)
	{
		for (int j=0; j<ColNum24; j++)
			brush[i][j].DeleteObject();
	}
}


// CGraDlg message handlers


void CGraDlg::OnClickedBtnReadrow()
{
	// TODO: Add your control notification handler code here

	// update PCRMask status
	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	if (PCRMask == 0)
	{
		AfxMessageBox("Please check the PCR mask");
		return;
	}
	else
	{
		// initial cycle number
		PCRCycCnt =0;
		
		// polling PCR mask
		int i=0;
		while(PollingPCRMask)
		{
			if (PCRMask & (0x01 << i))
			{
				PollingPCRMask = FALSE;		// stop polling
				PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
				PCRMask &= 0xFF << (i+1);		// ÒÑÑ¡PCRÎ»ÇåÁã
			}
			else
				i++;
		}
		PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

		// ÅÐ¶ÏÊÇ²»ÊÇµÚÒ»´Î°´»­ÐÐ°´Å¥
		if (DrawRowStart)
		{
			bBmp_Row.DeleteObject();
			bgDC_Row.SelectObject(oldBMP_Row);
			bgDC_Row.DeleteDC();
		}
		DrawRowStart = TRUE;

		CString sRownum;
		GetDlgItemText(IDC_EDIT_READROW,sRownum);
		GraBuf[0] = atoi(sRownum);

		// initial background DC
		RowDC = GetDlgItem(IDC_PCR2BMP) -> GetDC();
		bgDC_Row.CreateCompatibleDC(RowDC);

		bBmp_Row.CreateCompatibleBitmap(RowDC, arraysize, arraysize);
		oldBMP_Row = bgDC_Row.SelectObject(&bBmp_Row);
		bgDC_Row.SetBkColor(RGB(0,0,0));

		// start draw row
		PCRType = (PCRCycCnt<<4)|(0x01);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

		GraFlag = SENDGRAMSG;
		MakeGraPacket(0x02,PCRType,GraBuf[0]);
		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

	}
}

void CGraDlg::cycleRow12(void)
{
	// initial cycle number
	PCRCycCnt =0;

	// polling PCR mask
	int i=0;
	while(PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
			PCRMask &= 0xFF << (i+1);		// ÒÑÑ¡PCRÎ»ÇåÁã
		}
		else
			i++;
	}
	PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

	PCRType = (PCRCycCnt<<4)|(0x01);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

	GraFlag = SENDGRAMSG;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


void CGraDlg::OnBnClickedBtnDprow24()
{
	// TODO: Add your control notification handler code here

	// update PCRMask status
	GraFlag = GETPCRMASK;
	GraCalMainMsg();
	
	if (PCRMask == 0)
	{
		AfxMessageBox("Please check the PCR mask");
		return;
	}
	else
	{
		// initial cycle number
		PCRCycCnt =0;

		// polling PCR mask
		int i=0;
		while(PollingPCRMask)
		{
			if (PCRMask & (0x01 << i))
			{
				PollingPCRMask = FALSE;		// stop polling
				PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
				PCRMask &= 0xFF << (i+1);		// ÒÑÑ¡PCRÎ»ÇåÁã
			}
			else
				i++;
		}
		PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

		// ÅÐ¶ÏÊÇ²»ÊÇµÚÒ»´Î°´»­ÐÐ°´Å¥
		if (DrawRowStart)
		{
			bBmp_Row.DeleteObject();
			bgDC_Row.SelectObject(oldBMP_Row);
			bgDC_Row.DeleteDC();
		}
		DrawRowStart = TRUE;

		CString sRownum;
		GetDlgItemText(IDC_EDIT_READROW,sRownum);
		GraBuf[0] = atoi(sRownum);

		// initial background DC
		RowDC = GetDlgItem(IDC_PCR2BMP) -> GetDC();
		bgDC_Row.CreateCompatibleDC(RowDC);

		bBmp_Row.CreateCompatibleBitmap(RowDC, arraysize, arraysize);
		oldBMP_Row = bgDC_Row.SelectObject(&bBmp_Row);
		bgDC_Row.SetBkColor(RGB(0,0,0));

		// start draw row
		PCRType = (PCRCycCnt<<4)|(0x07);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

		GraFlag = SENDGRAMSG;
		MakeGraPacket(0x02,PCRType,GraBuf[0]);
		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

	}	
}


void CGraDlg::cycleRow24(void)
{
	// initial cycle number
	PCRCycCnt =0;

	// polling PCR mask
	int i=0;
	while(PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
			PCRMask &= 0xFF << (i+1);		// ÒÑÑ¡PCRÎ»ÇåÁã
		}
		else
			i++;
	}
	PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

	PCRType = (PCRCycCnt<<4)|(0x07);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

	GraFlag = SENDGRAMSG;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}


//Start Video: repurpose to Temp polling

BOOL g_CycPollingFlag = false;

void CGraDlg::OnBnClickedBtnDpvedio()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	g_CycPollingFlag = true;			// polling flag
	SetTimer(1,500,NULL);				// 500ms polling interval
}


void CGraDlg::OnBnClickedBtnDpvideo24()
{
	// TODO: Add your control notification handler code here
/*
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else if(m_GainMode == 1) {
		SetGainMode(0);
	}
	else {
		MessageBox("Video in HDR mode not allowed");
		return;
	}

	GraFlag = SENDVIDEOMSG;
	Vedio12_24Flag = 1;		// 24ÐÐvedio
	Gra_videoFlag = true;	// video¿ªÊ¼Ñ­»·
	Gra_pageFlag = TRUE;	// »­Ò³¿ªÊ¼Ñ­»·

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
*/
//	OnBnClickedButtonCapture();

//	SetTimer(1,VEDIOPITCHTIME,NULL);		// 12ÐÐvideo

}


//Stop Video Repurposed to stop temp recording

void CGraDlg::OnBnClickedBtnStopvideo()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if (cyc_status) {
		MessageBox("Cannot stop temp recording while cycler active.");
		return;
	}
	
	g_CycPollingFlag = false;
	KillTimer(1);		// stop video
}


void CGraDlg::OnBnClickedBtnClrtmprec()
{
	// TODO: Add your control notification handler code here

	CString str;
	str.Empty();

	m_Pelt = str;		//
	SetDlgItemTextA(IDC_EDIT_PELT, str);	// edit display

	m_Lidt = str;		// 
	SetDlgItemTextA(IDC_EDIT_LIDT, str);	// edit display
}



void CGraDlg::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here
	//reset picture data
	for (int i = 0; i < MAX_CHAN; i++) {
		for (int j = 0; j < numWells; j++) {
			m_yData[i][j].clear();
		}
//		m_factorInt[i].clear();
	}

	m_PixelData.Empty();
	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
}


void CGraDlg::OnBnClickedBtnAdcconvert()
{
	// TODO: Add your control notification handler code here
	char ch[2] = {'a','b'};
	TRACE("%s",ch);
}

//Clear ADC buffer, also clear m_yData buffer

void CGraDlg::OnBnClickedBtnClearadc()
{
	// TODO: Add your control notification handler code here
	//reset picture data

	for (int i = 0; i < MAX_CHAN; i++) {
		for (int j = 0; j < numWells; j++) {
			m_yData[i][j].clear();
		}
//		m_factorInt[i].clear();
	}

	m_ADCRecdata.Empty();
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
}

// Save the rec (hex) data to file - not used

// Repurposed to save DP file

void CGraDlg::OnBnClickedButton1()
{
	CString str;
	
	CFileDialog saveDlg(FALSE, ".ini",
		NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""INI(*.ini)|*.ini|""All Files(*.*)|*.*||",
		NULL, 0, TRUE);

	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		str = g_pDPReader->WriteDPIniFile();	
		ofs << str;
	}

/*	CString str;
	// ´´½¨Áí´æ¶Ô»°¿ò
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"ÎÄ±¾ÎÄ¼þ(*.txt)|*.txt|""ÎÄµµÎÄ¼þ(*.doc)|*.doc|""±í¸ñÎÄ¼þ(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// Áí´æ¶Ô»°¿òÄ¬ÈÏÂ·¾¶ÎªcÅÌ

	// ½øÐÐ±£´æ¶¯×÷
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// »ñÈ¡Òª±£´æ±à¼­¿ò¿Ø¼þÄÚµÄÊý¾Ý
		// IDC_EDIT_FILEÊÇ±à¼­¿ò¿Ø¼þ¾ä±ú
		pst->GetWindowTextA(str);
		ofs << str;
	}
*/
}

// Save the ADC data to file

void CGraDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;
	// ´´½¨Áí´æ¶Ô»°¿ò
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// Áí´æ¶Ô»°¿òÄ¬ÈÏÂ·¾¶ÎªcÅÌ

	// ½øÐÐ±£´æ¶¯×÷
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_ADCDATA);	// »ñÈ¡Òª±£´æ±à¼­¿ò¿Ø¼þÄÚµÄÊý¾Ý
		// IDC_EDIT_FILEÊÇ±à¼­¿ò¿Ø¼þ¾ä±ú
		pst->GetWindowTextA(str);
		ofs << str;
	}
}


void CGraDlg::OnBnClickedRadioAdcdata()
{
	// TODO: Add your control notification handler code here
}

// Not used any more

void CGraDlg::SetGainMode(int gain)
{
	if (gain == gain_mode) return;

/*	GraFlag = SENDGRAMSG;

	if (!gain)
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
//		GraCalMainMsg();
		gain_mode = 0;
	}
	else	
	{	
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x01;	//real data, date edit second byte
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
//			GraCalMainMsg();
		gain_mode = 1;
	}

	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
*/



#ifdef DARK_MANAGE

	// Zhimin Ding added: set V20 for each sensor individually

	if(g_pTrimDlg && g_pTrimReader) {
		g_pTrimDlg->SetGainMode(gain, 1);
		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[0].auto_v20[1], 1); 
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[0].auto_v20[0], 1); 

		g_pTrimDlg->SetGainMode(gain, 2);
		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[1].auto_v20[1], 2); 
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[1].auto_v20[0], 2); 

		g_pTrimDlg->SetGainMode(gain, 3);
		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[2].auto_v20[1], 3); 
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[2].auto_v20[0], 3); 

		g_pTrimDlg->SetGainMode(gain, 4);
		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[3].auto_v20[1], 4); 
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[3].auto_v20[0], 4); 
	}

#endif

}


void CGraDlg::DisplayHDR(void)
{	
	CDC *pDC;		//´´½¨Ä¿±êDCÖ¸Õë
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	

	CRect   rect;
	CBrush brush[12][12];	
	int i, j;
	int gray_level;
	int pdata[12][12];

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-100)*8 + 100;
		}
	}

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		
		}
	}

	for(i=0; i<12; i++) {
			for(j=0; j<12; j++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize12*j,pixelsize12*i,pixelsize12*(j+1),pixelsize12*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û?	

	for (i=0; i<12; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<12; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
			//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}


void CGraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

//	if(gain_mode <= 1) return; // only works for HDR mode.

	int pos;

/*	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);  

    // Check which slider sent the notification  
    if (pSlider == &c_Slider1)  
    {  
    }
    else if (pSlider == &c_Slider2)  
    {  
    }  
*/
    // Check what happened  
    switch(nSBCode)
    {
    case TB_LINEUP:  
    case TB_LINEDOWN:  
    case TB_PAGEUP:  
    case TB_PAGEDOWN:  
    case TB_THUMBPOSITION: 
		pos = nPos;
		contrast = 16 - pos/7;
//		if(!frame_size)
//			DisplayHDR();
//		else
//			DisplayHDR24();
		break;
    case TB_TOP:  
    case TB_BOTTOM:  
    case TB_THUMBTRACK:  
    case TB_ENDTRACK:  
    default:  
        break;  
    }

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGraDlg::DisplayHDR24(void)
{
	CDC *pDC;		//´´½¨Ä¿±êDCÖ¸Õë
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	

	CRect   rect;
	CBrush brush[24][24];	
	int i, j;
	int gray_level;
	int pdata[24][24];

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-100)*8 + 100;
		}
	}

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		
		}
	}

	for(i=0; i<24; i++) {
			for(j=0; j<24; j++)		// l´ú±íÁÐºÅ
			{
				rect.SetRect(pixelsize24*j,pixelsize24*i,pixelsize24*(j+1),pixelsize24*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// ×îºóÏÔÊ¾µÄADCÊý¾Ý×Ö·û?	

	for (i=0; i<24; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<24; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//½«½á¹û×ª³É×Ö·û´®ÏÔÊ¾
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//Ã¿ÐÐÊý¾Ý½«¼Ó»Ø³µ
			//×îÐÂÊý¾ÝÔÚ±à¼­¿ò×îºóÒ»ÐÐÏÔÊ¾
			//±à¼­¿ò´¹Ö±¹ö¶¯µ½µ×¶Ë
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}


//void CGraDlg::OnClickedRadiolowgain()
//{
//	// TODO: Add your control notification handler code here
//	UpdateData(true);
//}

/*
void CGraDlg::OnRadiohighgain()
{
	// TODO: Add your command handler code here
	UpdateData(true);
}


void CGraDlg::OnRadiohdr()
{
	// TODO: Add your command handler code here
	UpdateData(true);
}
*/

//void CGraDlg::OnClickedRadio12()
//{
//	// TODO: Add your control notification handler code here
//	UpdateData(true);
//
//	GraFlag = SENDGRAMSG;
//
//	TxData[0] = 0xaa;		//preamble code
//	TxData[1] = 0x01;		//command  TXC
//	TxData[2] = 0x05;		//data length
//	TxData[3] = 0x25;		//data type
//	TxData[4] = 0x00;		
//	TxData[5] = 0x00;	
//	TxData[6] = 0x00;       
//	TxData[7] = 0x00;	
//	for (int i=1; i<8; i++)
//		TxData[8] += TxData[i];
//	if (TxData[8]==0x17)
//		TxData[8]=0x18;
//	else
//		TxData[8]=TxData[8];
//	TxData[9]=0x17;
//	TxData[10]=0x17;
//
//	//Send message to main dialog
//	GraCalMainMsg();	//µ÷ÓÃÖ÷¶Ô»°¿ò´¦ÀíÏûÏ¢³ÌÐò		
//}


/*void CGraDlg::OnRadio24()
{	
	// TODO: Add your command handler code here
	UpdateData(true);

	GraFlag = SENDGRAMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x25;		//data type
	TxData[4] = 0x01;		
	TxData[5] = 0x00;	
	TxData[6] = 0x00;       
	TxData[7] = 0x00;	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	GraCalMainMsg();	//µ÷ÓÃÖ÷¶Ô»°¿ò´¦ÀíÏûÏ¢³ÌÐò
		
}
*/

void CGraDlg::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}
	else if (ReceiveTemCycNum > 0)
	{
		AfxMessageBox("No manual capture when temp cycle in progress!");
		return;
	}

	CaptureFrame(true);
}

void CGraDlg::CaptureFrame(BOOL commit)
{
	if (g_pTrimDlg && ReceiveTemCycNum == 0 && commit) {
		g_pTrimDlg->CommitTrim();
	}

	// Update PCRMask from GUI
	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	if (PCRMask == 0)
	{
		AfxMessageBox("Please check the channel mask - no channel selected");
		return;
	}

		// initial cycle flag
//		PCRCycCnt = 0;			no need to do it here.

		// ÅÐ¶ÏÊÇ²»ÊÇ°´µÚÒ»´Î»æÍ¼°´Å¥£¨capture£©
		if (DrawPageStart)
		{
			bBmp.DeleteObject();
			bgDC.SelectObject(oldBMP);
			bgDC.DeleteDC();
		}
		DrawPageStart = TRUE;

		// initial background DC 
		dispalyDC = GetDlgItem(IDC_Bmp) -> GetDC();
		bgDC.CreateCompatibleDC(dispalyDC);

		int DPI;
		DPI = bgDC.GetDeviceCaps(LOGPIXELSX);
		pixelsize12 = 16 * DPI / 96;
		pixelsize24 = 8 * DPI / 96;
		arraysize = pixelsize12 * 24;

		bBmp.CreateCompatibleBitmap(dispalyDC, arraysize, arraysize);
		oldBMP = bgDC.SelectObject(&bBmp);
		bgDC.SetBkColor(RGB(0,0,0));

		//-----------------------------------------------------
		// ½¨Á¢±à¼­¿ò±£´æµÄÎÄ¼þ
		//-----------------------------------------------------

#ifdef SAVE_DATA_FILE

		CString sRecDataFileName,sADCDataFileName;
		sRecDataFileName.Empty();
		sADCDataFileName.Empty();

		// °´µ±Ê±Ê±¼ä´´½¨ÎÄ¼þÃû
		CString sTime_Gra;					// system string format
		sTime_Gra.Empty();
		sysTime_Gra = CTime::GetCurrentTime();
		sTime_Gra.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
			sysTime_Gra.GetYear(),sysTime_Gra.GetMonth(),sysTime_Gra.GetDay(),
			sysTime_Gra.GetHour(),sysTime_Gra.GetMinute(),sysTime_Gra.GetSecond());

		sTime_Gra = ADCfilename.IsEmpty() ? sTime_Gra : ADCfilename;			// if the name already exists, continue use it.

		sRecDataFileName = "ImgData\\Log_Data_" + sTime_Gra + ".txt";
		sADCDataFileName = "ImgData\\Fluorescence_Data_" + sTime_Gra + ".txt";

		if (m_recdataFile.m_hFile != CFile::hFileNull)
			m_recdataFile.Close();

		if (m_adcdataFile.m_hFile != CFile::hFileNull)
			m_adcdataFile.Close();

//		m_recdataFile.Open(sRecDataFileName,CFile::modeCreate|CFile::modeWrite);
//		m_adcdataFile.Open(sADCDataFileName,CFile::modeCreate|CFile::modeWrite);

		m_recdataFile.Open(sRecDataFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);
		m_adcdataFile.Open(sADCDataFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite);

#endif

//		if (!m_GainMode) {
//			SetGainMode(1);
//		}
//		else {
//			SetGainMode(0);
//		}
/*
		// ·¢ËÍÏÔÊ¾Ö¸Áî
		if(!m_FrameSize) {
			typePixelSize = 12;
			if(m_GainMode <= 1) {
				CaptureFrame2();
			}
//			else {
//				OnBnClickedBtnDppage12();	//!!!!!Ã»ÓÐÓÃµ½£¬¾ÉµÄÑ­»··¢ËÍÄ£Ê½
//			}
		}
		else {
			typePixelSize = 24;
			if(m_GainMode <= 1) {
				CaptureFrame2();			// Zhimin mod
			}
//			else {
//				OnBnClickedBtnDppage24();	//!!!!!Ã»ÓÐÓÃµ½£¬¾ÉµÄÑ­»··¢ËÍÄ£Ê½
//			}
			if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();
		}
*/

		if (!frame_size) {
			typePixelSize = 12;
		}
		else {
			typePixelSize = 24;
		}
				
		CaptureFrame2();
	
		if (frame_size && g_pTrimDlg)
		{
			g_pTrimDlg->ResetTxBin();
		}
}



void CGraDlg::CaptureFrame2(void)
{
/*	int i = 0;

	// initial cycle number
	PCRCycCnt = 0;

	// polling PCR mask
	while (PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
			PCRMask &= 0xFF << i + 1;		// ÒÑÑ¡PCRÎ»ÇåÁã
		}
		else
			i++;
	}

	PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

	if (!m_FrameSize) {
		PCRType = (PCRCycCnt << 4) | (0x02);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype
	}
	else {
		PCRType = (PCRCycCnt << 4) | (0x08);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype
	}

	GraFlag = SENDPAGEMSG;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	MakeGraPacket(0x02,PCRType,0xff);
	
	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

	while (Gra_pageFlag)	// µ±Ç°Ò³»¹Î´»­Íê	
	{
		//Êý¾Ý´«ÊäbufferÇåÁã
		memset(RxData, 0, sizeof(RxData));

		// ¶ÁÈ¡ÏÂÒ»ÐÐ
		GraFlag = READGRAMSG;
		GraCalMainMsg();
	}
*/

	while (PCRMask != 0)	// ËùÓÐËùÑ¡PCRÒÑ»­Íê
	{
		memset(RxData, 0, sizeof(RxData));
		cyclePages();	// ·¢ËÍÏÂÒ»¸ö»­Ò³Ö¸Áî
	}

	PCRNum = 0;
	OnPaint_Flag = DRAWPAGE;
	Invalidate();

	Gra_videoFlag = true;		// videoÄ£Ê½Ê±£¬¿ªÊ¼·¢ËÍÏÂÒ»´Î

	if (PollingGraMode)		// polling mode
	{
		// update PCRMask status
		GraFlag = GETPCRMASK;		// need to do this for every capture since PCRMask gets cleared while cycling pages
		GraCalMainMsg();

		DynamicUpdateIntTime();

		ReceiveTemCycNum++;

		// update display
/*		CString str;			
		str.Empty();
		str.Format("%d", ReceiveTemCycNum);
		SetDlgItemTextA(IDC_EDIT_CYCNUM, str);
*/

		DisplayReceiveTemCycNum(ReceiveTemCycNum);

		//==========Pop up curve show ============
		if (ReceiveTemCycNum == 3) {
			if (!pCVS) {
				pCVS = new CCurveShow(this);

				pCVS->Create(IDD_CURVESHOW, this);
				pCVS->ShowWindow(SW_SHOW);
			}
			else {
				pCVS->ShowWindow(SW_RESTORE);
				//		pCVS->SetActiveWindow();
				//		pCVS->BringWindowToTop();
				//		pCVS->SetForegroundWindow();
			}
		}
		else if (ReceiveTemCycNum > 3) {
			if (pCVS) {
				pCVS->OnBnClickedAllcurve();
			}
		}
		//========================================

		GraFlag = STARTTEMHID;			// calls OpearCheckPollingProcess
		GraCalMainMsg();
	}
}

void CGraDlg::cyclePages(void)
{
	// initial cycle number
	PCRCycCnt = 0;

	// polling PCR mask
	int i = 0;

	while (PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// Used to set the PCRType
			PCRMask &= 0xFF << (i + 1);	// Clear the selected channel bit. Zhimin comment, so the use of PCRMask is destructive
		}
		else
			i++;
	}

	PollingPCRMask = TRUE;		// Restore polling flag

	if (!frame_size) {
		PCRType = (PCRCycCnt << 4) | (0x02);	// Setting up the "PCRType"
	}
	else {
		PCRType = (PCRCycCnt << 4) | (0x08);	// Setting up the "PCRType"
	}

	GraFlag = SENDPAGEMSG;
	Gra_pageFlag = TRUE;	// Start cycling rows

	MakeGraPacket(0x02, PCRType, 0xff);

	//Send message to main dialog
	GraCalMainMsg();		//Message to main dialog

							//===============

	while (Gra_pageFlag)	// Current page not done yet
	{
		// Clear RxData buffer
		memset(RxData, 0, sizeof(RxData));

		// Read the next row
		GraFlag = READGRAMSG;

		GraCalMainMsg();
	}
}

/*
void CGraDlg::CaptureFrame24(void)
{
	// initial cycle number
	PCRCycCnt =0;

	// polling PCR mask
	int i=0;
	while(PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
			PCRMask &= 0xFF << i+1;		// ÒÑÑ¡PCRÎ»ÇåÁã
		}
		else
			i++;
	}
	PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else {
		SetGainMode(0);
	}

	if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();

	PCRType = (PCRCycCnt<<4)|(0x08);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

	GraFlag = SENDPAGEMSG;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò	
}

void CGraDlg::cyclePage24(void)
{
	// initial cycle number
	PCRCycCnt =0;

	// polling PCR mask
	int i=0;
	while(PollingPCRMask)
	{
		if (PCRMask & (0x01 << i))
		{
			PollingPCRMask = FALSE;		// stop polling
			PCRCycCnt = i;				// ÓÃÀ´¸øÏÂÎ»»úÉèÖÃPCRType(×¢Òâ£ºÏÂÎ»»ú¸÷PCRµÄÐòºÅ´Ó0¿ªÊ¼)
			PCRMask &= 0xFF << i+1;		// ÒÑÑ¡PCRÎ»ÇåÁã
		}
		else
			i++;
	}
	PollingPCRMask = TRUE;		// polling±êÖ¾¸´Î»

	PCRType = (PCRCycCnt<<4)|(0x08);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

	GraFlag = SENDPAGEMSG;
	Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}
*/

// HDR stuff

void CGraDlg::OnBnClickedBtnDppage12()		//!!!!!Ã»ÓÐÓÃµ½£¬¾ÉµÄÑ­»··¢ËÍÄ£Ê½
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4¸öPCRÒÀ´Î·¢ËÍ
	{
		PCRType = (PCRCycCnt<<4)|(0x02);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = SENDPAGEMSG;
		Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

		//===========

		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = SENDPAGEMSG;
		Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

		DisplayHDR();
	}

}

// HDR stuff
void CGraDlg::OnBnClickedBtnDppage24()		//!!!!!Ã»ÓÐÓÃµ½£¬¾ÉµÄÑ­»··¢ËÍÄ£Ê½
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4¸öPCRÒÀ´Î·¢ËÍ
	{
		PCRType = (PCRCycCnt<<4)|(0x08);	// ·Ö±ðÉèÖÃ4¸öPCRµÄtype

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = SENDPAGEMSG;
		Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

		//===========

		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = SENDPAGEMSG;
		Gra_pageFlag = TRUE;	// ¿ªÊ¼Ñ­»·

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò

		//		DisplayHDR24();
	}
}


void CGraDlg::OnClickedCheckPixout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnClickedCheckOrigout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


BOOL CGraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	OnPaint_Flag = 0;

//	((CButton*)GetDlgItem(IDC_LED1_EN))->SetCheck(true);
//	((CButton*)GetDlgItem(IDC_LED2_EN))->SetCheck(true);
//	((CButton*)GetDlgItem(IDC_LED3_EN))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_LED_INDV_EN))->SetCheck(true);
	
//	regDispatch();

	CString str;
	str.Empty();
	str.Format("%d", ReceiveTemCycNum);
	SetDlgItemTextA(IDC_EDIT_CYCNUM, str);

	((CButton*)GetDlgItem(IDC_GraDlg_Check_AutoSnap))->SetCheck(true);		// Autosnap enable enabled by default.

	CFont Font;

	Font.CreateFont(16,                            // Height
		0,                             // Width
		0,                             // Escapement
		0,                             // Orientation
		FW_BOLD,                       // Weight
		FALSE,                         // Italic
		FALSE,                          // Underline
		0,                             // StrikeOut
		ANSI_CHARSET,                  // CharSet
		OUT_DEFAULT_PRECIS,            // OutPrecision
		CLIP_DEFAULT_PRECIS,           // ClipPrecision
		DEFAULT_QUALITY,               // Quality
		DEFAULT_PITCH | FF_SWISS,      // PitchAndFamily
		"Arial");                     // Facename

//	((CEdit*)GetDlgItem(IDC_EDIT_CONC))->SetFont(&Font);
//	((CStatic*)GetDlgItem(IDC_STATIC_BOLD))->SetFont(&Font);

	((CButton*)GetDlgItem(IDC_BTN_STARTTHERM))->SetFont(&Font);
	((CButton*)GetDlgItem(IDC_BUTTON11))->SetFont(&Font);

	int sp = 80;		// Initial slider position at 80%
	contrast = 16 - sp / 7;

	CSliderCtrl   *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);			// Initial high contrast for testing ONT chip
	pSlidCtrl->SetPos(sp);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	switch (OnPaint_Flag)
	{
	case DRAWPAGE:
		{
			dispalyDC->BitBlt(0, 0, arraysize, arraysize, &bgDC, 0, 0, SRCCOPY);
			break;
		}
	case DRAWROW:
		{
			RowDC->BitBlt(0, 0, arraysize, arraysize, &bgDC_Row, 0 , 0, SRCCOPY);
			break;
		}
	default:
		break;
	}

	// Do not call CDialogEx::OnPaint() for painting messages
}

#define LID_TEMP_THRESHOLD 70

void CGraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case 1:		// ³ÖÐøvideoÃüÁî·¢ËÍ¡¢Êý¾Ý¶ÁÈ¡ // Poll temperature reading
		{
			if (timerCtrFlag && g_CycPollingFlag)
			{
				if (ReceiveTemCycNum == 0) {
					if (g_LidTemp < LID_TEMP_THRESHOLD && cyc_status == 2) {		// when in cooling down mode, check lid temp and change to 0 cyc_status mode.
						SetDlgItemTextA(IDC_BTN_STARTTHERM, "Test Cycler");
						cyc_status = 0;
					}
				}

				GraFlag = SENDGRAMSG;

				if (ping_pong) ping_pong = false;		// toggle first
				else ping_pong = true;

					TxData[0] = 0xaa;		//preamble code
					TxData[1] = 0x14; //  0x10;		//command Zhimin changed, use 0x14 instead
					TxData[2] = 0x0C;		//data length
					TxData[3] = 0x0d; //  0x02;		//data type, date edit first byte
//					TxData[4] = 0x02;		//real data
					TxData[4] = 0x0; //  ping_pong ? 0x01 : 0x02;	// ping_pong true: Lid temp.
					TxData[5] = 0x00;		//Ô¤ÁôÎ»
					TxData[6] = 0x00;
					TxData[7] = 0x00;
					TxData[8] = 0x00;
					TxData[9] = 0x00;
					TxData[10] = 0x00;
					TxData[11] = 0x00;
					TxData[12] = 0x00;
					TxData[13] = 0x00;
					TxData[14] = 0x00;
					TxData[15] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5] + TxData[6] + TxData[7] + TxData[8] + TxData[9]
						+ TxData[10] + TxData[11] + TxData[12] + TxData[13] + TxData[14];		//check sum
					if (TxData[15] == 0x17)
						TxData[15] = 0x18;
					else
						TxData[15] = TxData[15];
					TxData[16] = 0x17;		//back code
					TxData[17] = 0x17;		//back code
											//Send message to main dialog
				GraCalMainMsg();		//
			}
			break;
		}
	case 2:			// graphic HID polling
		{
			if ( timerCtrFlag & PollingGraTimerFlag )
			{
				GraFlag = SENDGRAMSG;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x15;		//command  TXC
				TxData[2] = 0x05;		//data length
				TxData[3] = 0x01;		//data type
				TxData[4] = 0x00;		
				TxData[5] = 0x00;	
				TxData[6] = 0x00;       
				TxData[7] = 0x00;	
				for (int i=1; i<8; i++)
					TxData[8] += TxData[i];
				if (TxData[8]==0x17)
					TxData[8]=0x18;
				else
					TxData[8]=TxData[8];
				TxData[9]=0x17;
				TxData[10]=0x17;

				//Send message to main dialog
				GraCalMainMsg();	//µ÷ÓÃÖ÷¶Ô»°¿ò´¦ÀíÏûÏ¢³ÌÐò
			}
			break;
		}
	case 3: 
		{
			int done = AutocalibInt();
//			if (done) KillTimer(3);
			break;
		}
	case 4:
	{
		GraFlag = SENDGRAMSG;	// Test lid and main power

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x17;		//command  TXC
		TxData[2] = 0x01;		//data length
		TxData[3] = 0x01;		//data type
		for (int i = 1; i<4; i++)
			TxData[4] += TxData[i];
		if (TxData[4] == 0x17)
			TxData[4] = 0x18;

		TxData[5] = 0x17;
		TxData[6] = 0x17;

		//Send message to main dialog
		GraCalMainMsg();	//

		//===================
		break;
	}
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CGraDlg::OnClickedGradlgCheckAutosnap()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_GraDlg_Check_AutoSnap);
/*
	if(pCheckbox->GetCheck()) {
		PollingGraMode = TRUE;
		PollingGraTimerFlag = TRUE;
		SetTimer(2,PGRATIMERVALUE,NULL);		// start polling
	}
	else {
		PollingGraMode = FALSE;
		PollingGraTimerFlag = FALSE;
		KillTimer(2);		// stop polling
	}
*/

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x28;		//data type, date edit first byte
	TxData[4] = (pCheckbox->GetCheck())? 1:0;		//real data, date edit second byte

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code
	GraFlag = SENDGRAMSG;

	//Send message to main dialog
	GraCalMainMsg();
}


void CGraDlg::OnBnClickedLedIndvEn()
{
	// TODO: Add your control notification handler code here
	if(((CButton*)GetDlgItem(IDC_LED_INDV_EN))->GetCheck()==BST_CHECKED)
	{
	   ((CButton*)GetDlgItem(IDC_LED2_EN))->SetCheck(false);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->SetCheck(false);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->SetCheck(false);

	   ((CButton*)GetDlgItem(IDC_LED2_EN))->EnableWindow(true);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->EnableWindow(true);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->EnableWindow(true);
	}
	else
	{   
	   ((CButton*)GetDlgItem(IDC_LED2_EN))->EnableWindow(false);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->EnableWindow(false);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->EnableWindow(false);
	}
}


void CGraDlg::OnBnClickedLedSwitch()
{
		if(!g_DeviceDetected) {
			MessageBox("PDx16 Device Not Attached");
			return;
		}

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x23;		//data type, date edit first byte

		if(((CButton*)GetDlgItem(IDC_LED_INDV_EN))->GetCheck()==BST_UNCHECKED)
		{
			
			TxData[4] = (((CButton*)GetDlgItem(IDC_LED1_EN))->GetCheck()==BST_CHECKED)? 1:0;		//real data, date edit second byte
		}
		else
		{
			TxData[4] = 0x80;
			if((((CButton*)GetDlgItem(IDC_LED1_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 1;
			if((((CButton*)GetDlgItem(IDC_LED2_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 2;
			if((((CButton*)GetDlgItem(IDC_LED3_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 4;
			if((((CButton*)GetDlgItem(IDC_LED4_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 8;
		}
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		GraFlag = SENDGRAMSG;    
		//Send message to main dialog
		GraCalMainMsg();		//µ÷ÓÃÖ÷¶Ô»°¿ò´®¿Ú·¢ËÍÏûÏ¢³ÌÐò
}

// Copy status log instead

void CGraDlg::OnBnClickedBtnCopy()
{
	// Copy ADC data to clip board

	CEdit *myEdit;

//	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_ADCDATA);
	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_PELT);
	myEdit->SetSel(0, -1, FALSE);
	myEdit->Copy();
}


void CGraDlg::OnBnClickedBtnCopy2()
{
	// TODO: Add your control notification handler code here
	CEdit *myEdit;

	//	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_ADCDATA);
	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_LIDT);
	myEdit->SetSel(0, -1, FALSE);
	myEdit->Copy();
}



void CGraDlg::OnBnClickedPcrcurve()
{
//	CCurveShow dlgCurve;
//	if(dlgCurve.DoModal()==IDOK)
//	{
		
//	}
	
	if (!pCVS) {
		pCVS = new CCurveShow(this);

		pCVS->Create(IDD_CURVESHOW, this);
		pCVS->ShowWindow(SW_SHOW);
//		pCVS->SetGraDlg(this);
	}
	else {
		pCVS->ShowWindow(SW_RESTORE);
//		pCVS->SetActiveWindow();
//		pCVS->BringWindowToTop();
//		pCVS->SetForegroundWindow();
	}
}

// Read dataposition.ini and put in m_PositionChip
/*
void CGraDlg::regDispatch()
{
	CString strValue;
	CString path;
	TCHAR CurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,CurrentDirectory);
	path = CurrentDirectory;
	path += "\\Trim\\dataposition.ini";
	std::regex pattern("[A-Za-z0-9+]+");
	std::smatch result;

	CString strMatch;
	std::string str;
	std::string::const_iterator it;//È¡Í· 
	std::string::const_iterator end;//È¡Î²

	// Find the number of wells

	strMatch.Format("NWELLS");
	GetPrivateProfileString("PLATE CONFIG", strMatch, NULL, strValue.GetBuffer(254), 254, path);

	str = strValue.GetBuffer();

	if (std::regex_search(str, result, pattern))
	{
		numWells = atoi(result[0].str().c_str());
	}

	if (numWells > MAX_WELL)
	{
		CString mess;
		mess.Format("Number of wells cannot exceed %d", MAX_WELL);
		numWells = MAX_WELL;
		MessageBox(mess);
	}

	// Find the number of channels

	strMatch.Format("NCHANNELS");
	GetPrivateProfileString("PLATE CONFIG", strMatch, NULL, strValue.GetBuffer(254), 254, path);

	str = strValue.GetBuffer();

	if (std::regex_search(str, result, pattern))
	{
		numChannels = atoi(result[0].str().c_str());
	}

	if (numChannels > MAX_CHAN)
	{
		CString mess;
		mess.Format("Number of channels cannot exceed %d", MAX_CHAN);
		numWells = MAX_CHAN;
		MessageBox(mess);
	}

	// System ID

	strMatch.Format("SN1");
	GetPrivateProfileString("SYSTEM ID", strMatch, NULL, strValue.GetBuffer(254), 254, path);

	str = strValue.GetBuffer();

	if (std::regex_search(str, result, pattern))
	{
		Sn1 = atoi(result[0].str().c_str());

		if (Sn1 > 256 || Sn1 < 1)
		{
			CString mess;
			mess.Format("Serial number must be 1-255, currently: %d", Sn1);
			Sn1 = 0;
			MessageBox(mess);
		}
	}


	strMatch.Format("SN2");
	GetPrivateProfileString("SYSTEM ID", strMatch, NULL, strValue.GetBuffer(254), 254, path);

	str = strValue.GetBuffer();

	if (std::regex_search(str, result, pattern))
	{
		Sn2 = atoi(result[0].str().c_str());
		if (Sn2 > 256 || Sn2 < 1)
		{
			CString mess;
			mess.Format("Serial number must be 1-255, currently: %d", Sn2);
			Sn2 = 0;
			MessageBox(mess);
		}
	}

	strMatch.Format("MODEL");
	GetPrivateProfileString("SYSTEM ID", strMatch, NULL, strValue.GetBuffer(254), 254, path);

	str = strValue.GetBuffer();

	if (std::regex_search(str, result, pattern))
	{
		ModelLetter = str.at(0);
	}
	else {
		str = "x";
	}

	CString mess;
	mess.Format("Read from Dataposition file: System ID is: %s-%d-%d", str.c_str(), Sn1, Sn2);
	if(Sn1 > 0 && Sn1 < 256 && Sn2 > 0 && Sn2 < 256) 
		MessageBox(mess);

	// Find data position

	int nw;

	for(int i=0;i<4;i++)
	{
		strMatch.Format("CHIP%d",i+1);
		GetPrivateProfileString("DATA POSITION",strMatch,NULL,strValue.GetBuffer(254),254,path);
		
		str = strValue.GetBuffer();
		it=str.begin();//È¡Í· 
		end=str.end();//È¡Î²

		nw = 0;

		if(std::regex_search(str,result,pattern)) 
		{
			while (std::regex_search(it,end,result,pattern))
			{
				m_PositionChip[i].push_back(result[0]);//ÕâÀï¾ÍµÃµ½·Ö×é1
				it=result[0].second;  
				nw++;
			}
			if (nw < numWells) {
				MessageBox("Not enough data position in INI file for each well.");
			}
		}
	}
}
*/

// put polled temp reading in edit boxes.

// #define SHOW_PWM

vector <double> pelt_hist, lid_hist, aux_hist;
vector <int> per_hist, seg_hist;

BOOL lidopen = false;
int lid_count = 0;

BOOL mainpower_applied = true;
int powercycle_count = 0;

LRESULT CGraDlg::OnGraCycPollProcess(WPARAM wParam, LPARAM lParam)
{
	if (RxData[2] == 0x17) {
		CString str;

		BYTE lid_status = RxData[5] & 0x01;
		BYTE power_status = RxData[5] & 0x02;

		if (lid_status == 0) {
			if (lidopen) lid_count++;
			lidopen = false;

			str.Format("Lid closed: %d", lid_count);
			DisplayPollStatus(str);
		}
		else {
			lidopen = true;
			DisplayPollStatus("lid open");
		}

		if (power_status == 0) {
			if (!mainpower_applied) powercycle_count++;
			mainpower_applied = true;

			str.Format("main power on: %d", powercycle_count);
			DisplayPollStatus(str);
		}
		else {
			DisplayPollStatus("main power off");

			mainpower_applied = false;
		}

		return 0;
	}

	g_CycPollingFlag = false;		//

	CString sTem, sTem1, sTem2, sPwm, sPeriod;

	unsigned char cTem[] = { RxData[5],RxData[6],RxData[7],RxData[8] };
	unsigned char cTem1[] = { RxData[9],RxData[10],RxData[11],RxData[12] };
	unsigned char cTem2[] = { RxData[21],RxData[22],RxData[23],RxData[24] };
	unsigned char cPwm[] = { RxData[16],RxData[17] };

	float * fTem = (float *)cTem;
	float * fTem1 = (float *)cTem1;
	float * fTem2 = (float *)cTem2;
	//int		*iPwm = (int *)cPwm;

	int iPwm = (RxData[16] << 8) | RxData[17];
	unsigned char state = RxData[18];
	unsigned char period = RxData[19];
	unsigned char step = RxData[20];

	sTem.Format("%g", *fTem);
	sTem1.Format("%g", *fTem1);
	sTem2.Format("%g", *fTem2);
	sPwm.Format("%d", iPwm);
	sPeriod.Format("%d, %d, %d", state, period, step);

	unsigned char sindex = RxData[9];

	per_hist.push_back(period);
	seg_hist.push_back(step);

	//if (sindex == 2) {
		pelt_hist.push_back(*fTem1);

//		m_Pelt += (sTem1 + "\r\n");		// È¡³ö½ÓÊÕµÄfloatÊý¾Ý
		m_Pelt += (sTem1 + ", " + sPeriod + "\r\n");		// È¡³ö½ÓÊÕµÄfloatÊý¾Ý
//		m_Pelt += (sTem1 + ", " + sPwm + "\r\n");		// È¡³ö½ÓÊÕµÄfloatÊý¾Ý

		SetDlgItemTextA(IDC_EDIT_PELT, m_Pelt);	// edit display

		POINT ptPel;
		GetDlgItem(IDC_EDIT_PELT)->GetScrollRange(SB_VERT, (LPINT)&ptPel.x, (LPINT)&ptPel.y);
		ptPel.x = 0;
		GetDlgItem(IDC_EDIT_PELT)->SendMessage(EM_LINESCROLL, ptPel.x, ptPel.y);
	//}
	//else if(sindex == 1) {

#ifdef SHOW_PWM
		lid_hist.push_back((double)iPwm * 0.2);
		m_Lidt += (sPwm + "\r\n");
#else
		lid_hist.push_back(*fTem);

		if (RxData[3] > 0x11) m_Lidt += (sTem + "(" + sTem2 + ")\r\n");		// È¡³ö½ÓÊÕµÄfloatÊý¾Ý
		else m_Lidt += (sTem + "\r\n");
#endif

		aux_hist.push_back(*fTem2);

//		else m_Lidt += (sTem + "(" + sPeriod + ")\r\n");

		SetDlgItemTextA(IDC_EDIT_LIDT, m_Lidt);	// edit display

		// POINT ptPel;
		GetDlgItem(IDC_EDIT_LIDT)->GetScrollRange(SB_VERT, (LPINT)&ptPel.x, (LPINT)&ptPel.y);
		ptPel.x = 0;
		GetDlgItem(IDC_EDIT_LIDT)->SendMessage(EM_LINESCROLL, ptPel.x, ptPel.y);

		g_LidTemp = *fTem;
	//}

	g_CycPollingFlag = true;

	return 0;
}


// The main control button for thermal cycler

extern BOOL g_dirty;

void CGraDlg::OnBnClickedBtnStarttherm()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

#ifdef SAVE_DATA_FILE

	ADCfilename.Empty();
	CTime sysTime_Gra = CTime::GetCurrentTime();

	ADCfilename.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
		sysTime_Gra.GetYear(), sysTime_Gra.GetMonth(), sysTime_Gra.GetDay(),
		sysTime_Gra.GetHour(), sysTime_Gra.GetMinute(), sysTime_Gra.GetSecond());

#endif

	if (!cyc_status)
	{
		//========
		if (!g_CycPollingFlag) OnBnClickedBtnDpvedio();			// polling temperature
		//========

		for (int i = 0; i < MAX_CHAN; i++) {
			m_dynIntTime[i] = false;
			m_factorIntTime[i] = 1.0;

			m_maxPixVal[i] = DARK_LEVEL;
		}

		//reset picture data, by ych
		for (int i = 0; i < MAX_CHAN; i++) {
			for (int j = 0; j < numWells; j++) {
				m_yData[i][j].clear();
			}
			ifactor[i].clear();
			mtemp[i].clear();
		}

		pelt_hist.clear();
		lid_hist.clear();
		per_hist.clear();
		seg_hist.clear();
		aux_hist.clear();

		// Todo: Need to move init m_yData here.
		// Todo: need to clear ADC buffer.

		DisplayReceiveTemCycNum(ReceiveTemCycNum);

		SetDlgItemTextA(IDC_BTN_STARTTHERM, "Pause Cycler");
		cyc_status = 1;

		
		GraFlag = STARTTHERMCYC;
		GraCalMainMsg();	

		g_dirty = true;

		cycleStart = CTime::GetCurrentTime();

	}
	else if(cyc_status == 1) //running
	{
		int result = MessageBox("OK to abort?", "PCRProject", MB_ICONQUESTION | MB_OKCANCEL);

		if (result == IDOK) {
			SetDlgItemTextA(IDC_BTN_STARTTHERM, "Cooling down");
			cyc_status = 2;
			GraFlag = STOPTHERMCYC;
			GraCalMainMsg();

			ReceiveTemCycNum = 0;
			PollingGraTimerFlag = FALSE;
			PollingGraMode = FALSE;
			KillTimer(2);

			DisplayPollStatus("Abort, stop Gra polling (kill timer 2)");
		}
	}
}

// Called from PCRProjDlg->TEMHID_ReadHIDInputReport->CYCCMD, when cycle stopped naturally

#define ARRAY_DIM 2000

void CGraDlg::CoolingDown()
{
#ifdef SAVE_DATA_FILE
	ADCfilename.Empty();
#endif

	SetDlgItemTextA(IDC_BTN_STARTTHERM, "Cooling down");
	cyc_status = 2;

	ValidateTempCtrl();

	return;

	//================Eval temperture quality===================

	double x[ARRAY_DIM], xlp[ARRAY_DIM], y[ARRAY_DIM];

	int i;

	int n = pelt_hist.size();

	if (n > ARRAY_DIM)
		n = ARRAY_DIM;

	for (i = 0; i < n; i++) {
		x[i] = pelt_hist[i];
	}

	xlp[0] = x[0];

	for (i = 0; i < n-1; i++) {
		xlp[i + 1] = xlp[i] + 0.1 * (x[i + 1] - xlp[i]);
	}

	for (i = 0; i < n; i++) {
		y[i] = (x[i] - xlp[i]) * (x[i] - xlp[i]);
	}

	double ymax = y[0];
	int maxi = 0;

	for (i = 1; i < n; i++) {
		if (y[i] > ymax) {
			ymax = y[i];
			maxi = i;
		}
	}

	double ramp = sqrt(ymax) / 2.25; //  4.5; becaused now I used 0x14 0x0d commands.

	maxi += 100;

	double err = 0;

	for (i = 0; i < 10; i++) {
		err += (x[maxi + i] - 95) * (x[maxi + i] - 95);
	}

	err = sqrt(err) / 10;

	CString str, messyes, messno;

	str.Format("Maxramp: %3.2f(degree/s), Error %3.3f(degree)", ramp, err);

	DisplayPollStatus("Temp Control Test Status:");
	DisplayPollStatus(str);

	switch (currentLanguage) {
	case language_English:
		DisplayReportStatus("Temperature control test result: " + str);
		messyes = "Temperature control OK";
		messno = "Temperature control fail";
		break;
	case language_Chinese:
		DisplayReportStatus("温控测试结果： " + str);
		messyes = "温控测试通过·";
		messno = "温控测试不通过";
		break;
	}

	if (ramp > 8 && err < 0.3) {
		MessageBox(_T("Thermal Test Pass!"));
		DisplayReportStatus(messyes);
	}
	else {
		MessageBox(_T("Thermal Test Fail!"));
		DisplayReportStatus(messno);
	}

	//==========================================================
}

extern int my_rData;

// Display rData
void CGraDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here

	CString str;

	str.Format("rData=%d", my_rData);

	SetDlgItemTextA(IDC_EDIT3, str);
}

void CGraDlg::DisplayCyclerState(int state)
{
	// TODO: Add your control notification handler code here

	CString str;

	str.Format("Cyclr state = %d", state);

	SetDlgItemTextA(IDC_EDIT3, str);
}

extern int TemperCycNum;
extern int g_cycle_time;


void CGraDlg::DisplayReceiveTemCycNum(int num)
{
	// TODO: Add your control notification handler code here

	CString str;

	int remain = (TemperCycNum - num) * g_cycle_time;

	int h, m, s;

	h = remain / 3600;

	remain = remain % 3600;

	m = remain / 60;

	s = remain % 60;

	str.Format("%d (Remain: %.2d:%.2d:%.2d)", num, h, m, s);

	SetDlgItemTextA(IDC_EDIT_CYCNUM, str);
}

// Display poll status
void CGraDlg::DisplayPollStatus(CString status)
{
	if (status == m_lastStr) return;		// Do not display identicle messages.
	
	m_Poll += (status + "\r\n");			// 
	SetDlgItemTextA(IDC_EDIT6, m_Poll);		// 

	m_lastStr = status;

	POINT pt;
	GetDlgItem(IDC_EDIT6)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
	pt.x = 0;
	GetDlgItem(IDC_EDIT6)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
}

// Display poll status
void CGraDlg::DisplayReportStatus(CString status)
{
	m_Report += (status + "\r\n");			// 
	SetDlgItemTextA(IDC_EDIT8, m_Report);		// 

	POINT pt;
	GetDlgItem(IDC_EDIT8)->GetScrollRange(SB_VERT, (LPINT)&pt.x, (LPINT)&pt.y);
	pt.x = 0;
	GetDlgItem(IDC_EDIT8)->SendMessage(EM_LINESCROLL, pt.x, pt.y);
}

void CGraDlg::OnBnClickedButton14()
{
	// TODO: Add your control notification handler code here
	CString str;

	CFileDialog saveDlg(FALSE, ".txt",
		"Report_" + test_report.sys_id, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL, 0, TRUE);

	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT8);	//

		pst->GetWindowTextA(str);

		// str += u8"Add some Chinese characters: 加一些中文符号";

		ofs << str;
	}
}

//Debug get cycler state
void CGraDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here

	GraFlag = STARTTEMHID;
	GraCalMainMsg();
}

//Debug force stop cycler
void CGraDlg::OnBnClickedButton10()
{
	// TODO: Add your control notification handler code here

	GraFlag = STOPTHERMCYC;
	GraCalMainMsg();
}

// Test draw

void CGraDlg::OnBnClickedButton11()
{
	// TODO: Add your control notification handler code here

	OnAutocalibInt();

/*	CString str;

	str.Format("Chan %d, IntTime factor %3.2f", 1, m_factorIntTime[1]);
	DisplayPollStatus(str);

	if (DrawPageStart)
	{
		bBmp.DeleteObject();
		bgDC.SelectObject(oldBMP);
		bgDC.DeleteDC();
	}
	DrawPageStart = TRUE;

	// initial background DC 
	dispalyDC = GetDlgItem(IDC_Bmp)->GetDC();
	bgDC.CreateCompatibleDC(dispalyDC);

	int DPI;
	DPI = bgDC.GetDeviceCaps(LOGPIXELSX);

	pixelsize12 = 8 * DPI / 96;
	pixelsize24 = 4 * DPI / 96;
	arraysize = pixelsize12 * 24;

	bBmp.CreateCompatibleBitmap(dispalyDC, arraysize, arraysize);
	oldBMP = bgDC.SelectObject(&bBmp);

	int adc_level = 500;

	int gray_level = adc_level / contrast;
	if (gray_level > 255) gray_level = 255;
	else if (gray_level < 0) gray_level = 0;

	CBrush brush;
	brush.CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		// dark gray

	bgDC.SetBkColor(RGB(0, 0, 0));

	displayRect.SetRect(0, 0, arraysize, arraysize);
	bgDC.Rectangle(displayRect);
	bgDC.FillRect(&displayRect, &brush);

	brush.DeleteObject();

	OnPaint_Flag = DRAWPAGE;
	Invalidate();
*/
}


void CGraDlg::OnBnClickedCheckDelaytrigger()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}

BOOL g_Melt = false;

void CGraDlg::OnBnClickedCheckMelt()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	g_Melt = m_Melt;
}

float opt_int_time[4];
unsigned int autoRepeatCounter = 0;

void CGraDlg::OnAutocalibInt()
{
	// TODO: Add your control notification handler code here
	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	int result = MessageBox("Please insert tubes for calibration.", "PCRProject", MB_ICONQUESTION | MB_OKCANCEL);
	if (result != IDOK) return;

	//	m_PixelData = "";
	//	SetDlgItemText(IDC_EDIT_RecData, m_PixelData);

	// Update PCRMask from GUI
	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	int k = 0;

	while (PCRMask != 0)	//
	{
		int i = 0;
		BOOL scan = true;

		while (scan)
		{
			if (PCRMask & (0x01 << i))
			{
				scan = false;				// stop scanning
				k = i;
				PCRMask &= 0xFF << (i + 1);	// Clear the selected channel bit. Zhimin comment, so the use of PCRMask is destructive
			}
			else
				i++;
		}

		opt_int_time[k] = 1;

		if (g_pTrimDlg) {
			g_pTrimDlg->CommitTrim();
			g_pTrimDlg->SetIntTime(opt_int_time[k], k + 1);
			g_pTrimDlg->SetIntTimeGUI("1.0", k + 1);			// 1 based index.
		}
	}

	for (int i = 0; i < MAX_CHAN; i++) {
		outlier_pixel[i] = 0;
	}

	//====================

	SetTimer(3, 1000, NULL);		// 
}

int CGraDlg::AutocalibInt()
{
	int done = 0;

	CaptureFrame(false);						// read a row
	autoRepeatCounter++;

	// Update PCRMask from GUI
	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	int k = 0;
	int max_read;
	float inc;
	CString str, messyes, messno;

	int maxx_read[MAX_CHAN];

	while (PCRMask != 0)	//
	{
		int i = 0;
		BOOL scan = true;

		while (scan)
		{
			if (PCRMask & (0x01 << i))
			{
				scan = false;				// stop scanning
				k = i;
				PCRMask &= 0xFF << (i + 1);	// Clear the selected channel bit. Zhimin comment, so the use of PCRMask is destructive
			}
			else
				i++;
		}

		max_read = GetMaxChanRead(k);					// Max channel reading
														//		inc = (float)(2000 - max_read) * 0.006;			// slowly approach the opt int time to avoid saturation
		inc = (float)(2000 - max_read) * 0.0033;			// slowly approach the opt int time to avoid saturation, smaller step length.

															//		if (abs(inc) < 0.3)
															//			done = 1;
															//		else
		opt_int_time[k] += inc;

		if (opt_int_time[k] > 800)
			opt_int_time[k] = 800;

		if (g_pTrimDlg)
			g_pTrimDlg->SetIntTime(opt_int_time[k], k + 1);

		str.Format("Ch: %d, Max read: %d, int time: %0.2f, gain: %0.2f", k + 1, max_read, opt_int_time[k], max_read/ opt_int_time[k]);
		DisplayPollStatus(str);
		if (autoRepeatCounter > 15) DisplayReportStatus(str);

		maxx_read[k] = max_read;
	}

	if (autoRepeatCounter >= 16) {
		done = 1;
	}

	if (!done)
		return 0;

	autoRepeatCounter = 0;

	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	k = 0;

	while (PCRMask != 0)	//
	{
		int i = 0;
		BOOL scan = true;

		while (scan)
		{
			if (PCRMask & (0x01 << i))
			{
				scan = false;				// stop scanning
				k = i;
				PCRMask &= 0xFF << (i + 1);	// Clear the selected channel bit. Zhimin comment, so the use of PCRMask is destructive
			}
			else
				i++;
		}

		str.Format("%.1f", roundf(opt_int_time[k]));
		g_pTrimDlg->SetIntTimeGUI(str, k + 1);
	}

	DisplayPollStatus("Int Time Calibration Done");

	
	KillTimer(3);

	k = 0;

	switch (currentLanguage) {
	case language_English:
		
		messyes = "Optical system test pass!";
		messno = "Optical system test fail!";
		break;
	case language_Chinese:
		
		messyes = "光学系统测试通过·";
		messno = "光学系统测试不通过";
		break;
	}

	if (maxx_read[k] > 1000 && maxx_read[k] < 3000 && opt_int_time[k] < 400 && opt_int_time[k] > 3) { // && outlier_pixel[k] < 500 && outlier_pixel[k] > 50) {
		MessageBox(_T("Optical system test pass!"));
		DisplayReportStatus(messyes);
	}
	else {
		MessageBox(_T("Optical system test fail!"));
		DisplayReportStatus(messno);
	}

	return 1;
}

int CGraDlg::GetMaxChanRead(int ch)
{
	std::vector<double> yData = m_yData[ch][0];
	int size = yData.size();
	int max = yData[size - 1];

	for (int i = 1; i < numWells; i++) {
		yData = m_yData[ch][i];
		if (yData[size - 1] > max) {
			max = yData[size - 1];
		}
	}

	return max;
}

int addy = 0, addx = 0;

void CGraDlg::OnBnClickedCheckShowmarker()
{
	UpdateData(true);

	addx = 0;
	addy = 0;
	RefreshMarkers();
}

void CGraDlg::RefreshMarkers()
{
	// if (m_ShowMarker) addone++;

	if (DrawPageStart)
	{
		bBmp.DeleteObject();
		bgDC.SelectObject(oldBMP);
		bgDC.DeleteDC();
	}
	DrawPageStart = TRUE;

	// initial background DC 
	dispalyDC = GetDlgItem(IDC_Bmp)->GetDC();
	bgDC.CreateCompatibleDC(dispalyDC);

	int DPI;
	DPI = bgDC.GetDeviceCaps(LOGPIXELSX);
	pixelsize12 = 16 * DPI / 96;
	pixelsize24 = 8 * DPI / 96;
	arraysize = pixelsize12 * 24;

	bBmp.CreateCompatibleBitmap(dispalyDC, arraysize, arraysize);
	oldBMP = bgDC.SelectObject(&bBmp);
	bgDC.SetBkColor(RGB(0, 0, 0));

	CBrush brush[RowNum24][ColNum24];	// ÐÂ»­Ë¢


	// Update PCRMask from GUI
	GraFlag = GETPCRMASK;
	GraCalMainMsg();

	// Scan channels
	while (PCRMask != 0)	//
	{
		int k = 0;
		BOOL scan = true;

		while (scan)
		{
			if (PCRMask & (0x01 << k))
			{
				scan = false;				// stop scanning
				PCRNum = k + 1;
				PCRMask &= 0xFF << (k + 1);	// Clear the selected channel bit. Zhimin comment, so the use of PCRMask is destructive
			}
			else
				k++;
		}

		switch (PCRNum)
		{
		case 1:
			if (typePixelSize == 12)		// 12ÐÐÇé¿ö
			{
				xOffset = 0;
				yOffset = 0;
			}
			else		// 24ÐÐÇé¿ö
			{
				xOffset = 0;
				yOffset = 0;
			}
			break;
		case 2:
			if (typePixelSize == 12)
			{
				xOffset = pixelsize12 * 12;		// offset = pixelsize * rowNum		
				yOffset = 0;
			}
			else
			{
				xOffset = pixelsize24 * 24;
				yOffset = 0;
			}
			break;
		case 3:
			if (typePixelSize == 12)
			{
				xOffset = 0;
				yOffset = pixelsize12 * 12;
			}
			else
			{
				xOffset = 0;
				yOffset = pixelsize24 * 24;
			}
			break;
		case 4:
			if (typePixelSize == 12)
			{
				xOffset = pixelsize12 * 12;
				yOffset = pixelsize12 * 12;
			}
			else
			{
				xOffset = pixelsize24 * 24;
				yOffset = pixelsize24 * 24;
			}
			break;
		default:
			break;
		}

		int i = 0;
		int l = 0;
		int gray_level = 0;

		for (int rn = 0; rn < 12; rn++) {
			for (i = 0; i < ColNum12; i++)
			{
				gray_level = v_frame_data[PCRNum-1][rn].at(i) / contrast;
//				if (gray_level >= 5000) gray_level = 100;

				if (gray_level > 255) gray_level = 255;
				else if (gray_level < 0) gray_level = 0;

				brush[rn][i].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));				 // zd mod use corrected adc data with green tint																								 //		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+6],RxData[i*2+6],RxData[i*2+6]));		// È¡¸ßbyteÎªÓÐÐ§Î»
			}

			for (int l = 0; l < ColNum12; l++)		// 
			{
				displayRect.SetRect((pixelsize12*l + xOffset), 
					(pixelsize12*rn + yOffset), 
					(pixelsize12*(l + 1) + xOffset), 
					(pixelsize12*(rn + 1) + yOffset));
				
				bgDC.Rectangle(displayRect);
				bgDC.FillRect(&displayRect, &brush[rn][l]);
			}
		}

		//=========Draw markers================

		if (m_ShowMarker) {
			for (int frame = 0; frame < numWells; frame++)
			{
				int sz = g_pDPReader->row_index[PCRNum - 1][frame].size();
				int row, col, r_min = 23, c_min = 23, nwell;

				for (int i = 0; i < sz; i++)
				{
					g_pDPReader->col_index[PCRNum - 1][frame].at(i) += addx;
					g_pDPReader->row_index[PCRNum - 1][frame].at(i) += addy;

					row = g_pDPReader->row_index[PCRNum - 1][frame].at(i);
					col = g_pDPReader->col_index[PCRNum - 1][frame].at(i);

					nwell = g_pDPReader->num_wells;

					if (row < r_min) r_min = row;
					if (col < c_min) c_min = col;
				}

				for (int i = 0; i < sz; i++)
				{
					row = g_pDPReader->row_index[PCRNum - 1][frame].at(i);
					col = g_pDPReader->col_index[PCRNum - 1][frame].at(i);

					CString str;
					
					if (row == r_min && col == c_min && nwell > 4) {
						if (nwell == 8) {
							if (frame < 4) {
								str = "a";
							}
							else {
								str = "b";
							}
						}
						else {
							if (frame < 8) {
								str = "a";
							}
							else {
								str = "b";
							}
						}
					}
					else {
						int fn = frame;
						if (nwell == 8) {
							if (frame >= 4) {
								fn -= 4;
							}
						}
						else if (nwell == 16) {
							if (frame >= 8) {
								fn -= 8;
							}
						}							
						str.Format("%d", fn + 1);
					}

					displayRect.SetRect((pixelsize12*col) + xOffset,
						(pixelsize12*row) + yOffset,
						(pixelsize12*(col + 1)) + xOffset,
						(pixelsize12*(row + 1)) + yOffset);

					bgDC.SetTextColor(0x00EEEEEE);
					bgDC.DrawText(str, displayRect, DT_CENTER);
					bgDC.DrawEdge(displayRect, EDGE_ETCHED, BF_RECT);
				}
			}
		}
			
		// release brush memory
		for (int i = 0; i < RowNum24; i++)
		{
				for (int j = 0; j < ColNum24; j++)
					brush[i][j].DeleteObject();
		}
		//==============================
	}

	OnPaint_Flag = DRAWPAGE;
	Invalidate();
}


void CGraDlg::OnBnClickedCheck6()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (m_bTestLid) {
		SetTimer(4, 300, NULL);		// 
	}
	else {
		KillTimer(4);
	}
}


void CGraDlg::OnBnClickedButtonMarkerup()
{
	// TODO: Add your control notification handler code here
	addx = 0;
	addy = -1;
	RefreshMarkers();
}


void CGraDlg::OnBnClickedButtonMarkerleft()
{
	// TODO: Add your control notification handler code here
	addx = -1;
	addy = 0;
	RefreshMarkers();
}


void CGraDlg::OnBnClickedButtonMarkerdown()
{
	// TODO: Add your control notification handler code here
	addx = 0;
	addy = 1;
	RefreshMarkers();
}


void CGraDlg::OnBnClickedButtonMarkerright()
{
	// TODO: Add your control notification handler code here
	addx = 1;
	addy = 0;
	RefreshMarkers();
}

#define DSIZE 2000
#define DNAT_TEMP 95
#define ANNL_TEMP 60

void CGraDlg::ValidateTempCtrl()
{
	//================Eval temperture quality===================

	double x[DSIZE], xlp[DSIZE], xhp[DSIZE];
	int per[DSIZE], seg[DSIZE];

	int n = pelt_hist.size();

	// ASSERT(n < DSIZE);

	if (n > DSIZE)
		n = DSIZE;

	double dt = 0.507422;
	int i;

	CTime curTime = CTime::GetCurrentTime();
	CTimeSpan span = curTime - cycleStart;

	int elapse = span.GetTotalSeconds();
	dt = (double)elapse / (double)n;

	for (i = 0; i < n; i++) {
		x[i] = pelt_hist[i];
		per[i] = per_hist[i];
		seg[i] = seg_hist[i];
	}

	xlp[0] = x[0];

	for (i = 0; i < n - 1; i++) {
		xlp[i + 1] = xlp[i] + 0.1 * (x[i + 1] - xlp[i]) * dt;
	}

	for (i = 0; i < n; i++) {
		xhp[i] = (x[i] - xlp[i]) * (x[i] - xlp[i]);
	}

	double hpmax = xhp[0];
	int maxi = 0, tick1, tick2, tick3, mark;

	for (i = 1; i < n; i++) {
		if (xhp[i] > hpmax) {
			hpmax = xhp[i];
			maxi = i;
		}
	}

	CString str, s, messyes, messno;

	double ramp, pramp; //  = sqrt(hpmax) / 4.5;
	ramp = (x[maxi] - x[maxi - 12]) / (12 * dt);
	pramp = ramp;

	mark = maxi + 50; // about 25s after ramp

	//s.Format("\r\nmaxi %d \r\n", maxi);
	//str += s;

	double err = 0, perr;
	for (i = 0; i < 50; i++) {
		err += (x[mark + i] - DNAT_TEMP) * (x[mark + i] - DNAT_TEMP);
		s.Format("temp = %3.2f\r\n", x[mark + i]);
	//	str += s;
	}
	err = sqrt(err / 50);
	perr = err;

	s.Format("\r\nMaxramp: %3.2f(degree/s), Error %3.3f(degree), dt:  %3.3f(s)", ramp, err, dt);
	str += s;

	for (i = 0; i < n; i++) {
		mark = i;
		if (per[i] == 2)
			break;
	}

	tick1 = mark;

	ramp = (x[mark + 9] - x[mark - 1]) / (10 * dt);

	mark += 40; // about 20s include ramp time and overshoot
	err = 0;
	for (i = 0; i < 10; i++) {
		err += (x[mark + i] - DNAT_TEMP) * (x[mark + i] - DNAT_TEMP);
	}
	err = sqrt(err / 10);

	s.Format("\r\nPeriod2 ramp_up: %3.2f(degree/s), Error %3.3f(degree)", ramp, err);
	str += s;

	for (i = 0; i < n; i++) {
		mark = i;
		if (per[i] == 2 && seg[i] == 1)
			break;
	}

	tick2 = mark;

	ramp = (x[mark + 15] - x[mark + 1]) / (14 * dt);

	mark += 30; // about 15s include ramp time and overshoot
	err = 0;
	for (i = 0; i < 10; i++) {
		err += (x[mark + i] - ANNL_TEMP) * (x[mark + i] - ANNL_TEMP);
	}
	err = sqrt(err / 10);

	s.Format("\r\nPeriod2 ramp_down: %3.2f(degree/s), Error %3.3f(degree)", ramp, err);
	str += s;

	for (i = 0; i < n; i++) {
		mark = i;
		if (per[i] == 3)
			break;
	}

	tick3 = mark;

	double dnat_per = (double)(tick2 - tick1) * dt;
	double ann_per = (double)(tick3 - tick2) * dt;

	s.Format("\r\nDenature period: %3.2f(sec),  Annealing period: %3.2f(sec)", dnat_per, ann_per);
	str += s;

	//============

	double lramp, aramp; //
	ASSERT(maxi > 100);

	for (i = 0; i < n; i++) {
		x[i] = lid_hist[i];
	}

	int interval = maxi - 25 - 5;

	lramp = (x[maxi - 25] - x[5]) / (interval * dt);

	for (i = 0; i < n; i++) {
		x[i] = aux_hist[i];
	}

	aramp = (x[maxi - 25] - x[5]) / (interval * dt);

	s.Format("\r\nLid ramp_up: %3.2f(degree/s), aux ramp_up: %3.2f(degree/s)", lramp, aramp);
	str += s;

	//============

	DisplayPollStatus("Temp Control Test Status:");
	DisplayPollStatus(str);

	switch (currentLanguage) {
	case language_English:
		DisplayReportStatus("Temperature control test result: " + str);
		messyes = "Temperature control OK";
		messno = "Temperature control fail";
		break;
	case language_Chinese:
		DisplayReportStatus("温控测试结果： " + str);
		messyes = "温控测试通过·";
		messno = "温控测试不通过";
		break;
	}

	if (pramp > 5.5 && perr < 0.75) {
		MessageBox(_T("Thermal Test Pass!"));
		DisplayReportStatus(messyes);
	}
	else {
		MessageBox(_T("Thermal Test Fail!"));
		DisplayReportStatus(messno);
	}

	//==========================================================
}





