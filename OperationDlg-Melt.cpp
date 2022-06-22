// OperationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "OperationDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <afx.h>
#include <vector>

#include <fstream>
#include <afx.h>
#include <vector>
#include "json\include\json.h"	

#include "GraDlg.h"

#define CONFIGFILE "Config.json"

using namespace std;


//***************************************************************
//Global variable definition
//***************************************************************
// Parameter variable in edit
CString sPITem_write,sCycle;
CString sInidenTem_write,sInidenTim_write;
CString sDenTem_write,sDenTim_write,sDenRam_write;
CString sAnnTem_write,sAnnTim_write,sAnnRam_write;
CString sAnnTem2_write, sAnnTim2_write, sAnnRam2_write;
CString sExtTem_write,sExtTim_write,sExtRam_write;
CString sHoldTem_write,sHoldTim_write;
CString sSave;		// save parameter buffer

// Current cycler parameters
float	curPiTemp = 105;			// This reflect the current HW/FW value. 
int		numCycle = 50;
float	curInitDenTemp = 95;
int		curInitDenTime = 120;
float	curDenTemp = 95;
int		curDenTime = 15;
float	curDenRamp = 5;
float	curAnnTemp = 60;
int		curAnnTime = 20;
float	curAnnRamp = 2;
float	curAnnTemp2 = 60;
int		curAnnTime2 = 20;
float	curAnnRamp2 = 2;
float	curExtTemp = 60;
int		curExtTime = 20;
float	curExtRamp = 2;
float	curHoldTemp = 50;
int		curHoldTime = 10;
int		curANNCycle = 50;

float curOverTime = 5;
float curOverTemp = 4;
float curOverTime2 = 5;
float curOverTemp2 = 10;

int OperDlgFlag = 0;			// operation dialog message flag
int OperReadstautsFlag = 3;		// read status flag
int cntReadstatuspel = 0;		// read status edit clear counter for peliter
int cntReadstatuspi = 0;		// read status edit clear counter for pi heat
int cntReadstatuspwm = 0;		// read status edit clear counter for pwm
int TemperCycNum = 0;			// 温度循环次数
int ReceiveTemCycNum = 0;		// 温度循环接收到数据的次数
BOOL temReadTimerFlag = false;	// PI、Peltier温度循环读取timer,发送指令允许标志

int opeDlgTimerFlag = 0;		// timer flag

// Zhimin modified for PDx 16 3 channel Alpha version
// int PCRMask = 0x7;				// 被连接的PCR序号标志
								// bit0:PCR1; bit1:PCR2; bit2:PCR3; bit3:PCR4;
//int ck1=0;
//int ck2=0;
//int ck3=0;
//int ck4=0;

CBrush m_editBrush;				// edit back-color brush

BYTE OperBuf [200];				// operation dialog buffer

//CFile m_readPIFile(_T("Read_PEL_Data.txt"),CFile::modeCreate|CFile::modeWrite);			// read peliter file
//CFile m_readPELFile(_T("Read_PI_Data.txt"),CFile::modeCreate|CFile::modeWrite);			// read pi file

CFile m_readPIFile;				// read peliter file
CFile m_readPELFile;			// read pi file
CFile m_saveTeDataFile;			// save test ram data file

CTime sysTime;					// system time
int iYear,iMonth,iDay;			// year / month / day
int iHour,iMinute,iSecond;		// hour / minute / second
CString sTime;					// system string format

BOOL ReadTemBtnStatus = TRUE;

BOOL AutoSampleFlag = FALSE;	// opeDlg atuo_sample start flag

static int saveTeDataCnt = 0;	// save test ram data file counter

extern BOOL g_DeviceDetected;

//==============

extern std::vector<double> m_yData[MAX_CHAN][MAX_WELL];	//handled data，4 channels，4 frames
extern int numWells;

extern float log_threshold;


//***************************************************************
// Own function
//***************************************************************


// COperationDlg dialog

IMPLEMENT_DYNAMIC(COperationDlg, CDialog)

COperationDlg::COperationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COperationDlg::IDD, pParent)
	, m_EnableAnnealing2(TRUE)
	, m_EnableExtension(TRUE)
	, m_bMaxRamp(FALSE)
{

	m_operdlg_sEditpel = _T("");
	m_operdlg_sEditpiread = _T("");
	m_operdlg_sEditcycleread = _T("");
	//  m_operdlg_sEditpwm = _T("");
	m_operdlg_sEditvanstatus = _T("");
	m_operdlg_sEditcurstatus = _T("");
	//  m_operdlg_sEditpwm2 = _T("");
	//  m_operdlg_sEditpwm1 = _T("");
	m_operdlg_iCheckMask1 = 1;
	m__operdlg_iCheckMask2 = 1;
	m__operdlg_iCheckMask3 = 1;
	m__operdlg_iCheckMask4 = 0;
	m_opedlg_sEditpwm1 = _T("");
	m_opedlg_sEditpwm2 = _T("");
}

COperationDlg::~COperationDlg()
{
}

void COperationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OPE_PEL, m_operdlg_sEditpel);
	DDX_Text(pDX, IDC_EDIT_OPE_PIREAD, m_operdlg_sEditpiread);
	DDX_Text(pDX, IDC_EDIT_OPE_CYCLEREAD, m_operdlg_sEditcycleread);
	//  DDX_Control(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm);
	DDX_Text(pDX, IDC_EDIT_OPE_VANSTATUS, m_operdlg_sEditvanstatus);
	DDX_Text(pDX, IDC_EDIT_OPE_CURRENTSTATUS, m_operdlg_sEditcurstatus);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM2, m_operdlg_sEditpwm2);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm1);
	DDX_Control(pDX, IDC_BTN_OPE_READSTART, m_opeDlg_ReadStart);
	DDX_Check(pDX, IDC_CHECK_OPE_MASK1, m_operdlg_iCheckMask1);
	//  DDX_Check(pDX, IDC_CHECK_OPE_MASK2, m__operdlg_iCheckMask2);
	DDX_Check(pDX, IDC_CHECK_OPE_MASK2, m__operdlg_iCheckMask2);
	DDX_Check(pDX, IDC_CHECK_OPE_MASK3, m__operdlg_iCheckMask3);
	DDX_Check(pDX, IDC_CHECK_OPE_MASK4, m__operdlg_iCheckMask4);
	DDX_Text(pDX, IDC_EDIT_OPE_PWM1, m_opedlg_sEditpwm1);
	DDX_Text(pDX, IDC_EDIT_OPE_PWM3, m_opedlg_sEditpwm2);
	DDX_Check(pDX, IDC_CHECK_ANN2, m_EnableAnnealing2);
	DDX_Check(pDX, IDC_CHECK_ENEXT, m_EnableExtension);
	DDX_Check(pDX, IDC_CHECK_MAXRAMP, m_bMaxRamp);
	DDX_Control(pDX, IDC_CUSTOM_TCHART, m_TChart);
}


BEGIN_MESSAGE_MAP(COperationDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OPE_LOAD, &COperationDlg::OnBnClickedBtnOpeLoad)
	ON_BN_CLICKED(IDC_BTN_OPE_SAVESTATUS, &COperationDlg::OnBnClickedBtnOpeSavestatus)
	ON_BN_CLICKED(IDC_BTN_OPE_CLEAR, &COperationDlg::OnBnClickedBtnOpeClear)
//	ON_EN_CHANGE(IDC_EDIT_OPE_DENTEM, &COperationDlg::OnEnChangeEditOpeDentem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_PITEM, &COperationDlg::OnEnKillfocusEditOpePitem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENTEM, &COperationDlg::OnEnKillfocusEditOpeDentem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENTIME, &COperationDlg::OnEnKillfocusEditOpeDentime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENINGTEM, &COperationDlg::OnEnKillfocusEditOpeDeningtem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENINGTIME, &COperationDlg::OnEnKillfocusEditOpeDeningtime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTEM, &COperationDlg::OnEnKillfocusEditOpeAnntem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTIME, &COperationDlg::OnEnKillfocusEditOpeAnntime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNCYCLE, &COperationDlg::OnEnKillfocusEditOpeAnncycle)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_EXTTEM, &COperationDlg::OnEnKillfocusEditOpeExttem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_EXTTIME, &COperationDlg::OnEnKillfocusEditOpeExttime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_HOLDTEM, &COperationDlg::OnEnKillfocusEditOpeHoldtem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_HOLDTIME, &COperationDlg::OnEnKillfocusEditOpeHoldtime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KP, &COperationDlg::OnEnKillfocusEditOpeKp)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KI, &COperationDlg::OnEnKillfocusEditOpeKi)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KD, &COperationDlg::OnEnKillfocusEditOpeKd)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KL, &COperationDlg::OnEnKillfocusEditOpeKl)
ON_BN_CLICKED(IDC_BTN_OPE_SAVEPARA, &COperationDlg::OnBnClickedBtnOpeSavepara)
ON_BN_CLICKED(IDC_BTN_OPE_MORE, &COperationDlg::OnBnClickedBtnOpeMore)
ON_MESSAGE(UM_OPERPROCESS,OnOpearProcess)
ON_MESSAGE(UM_OPERPIDPROCESS,OnOpearPIDProcess)
ON_MESSAGE(UM_OPERLEDPROCESS,OnOpearLEDProcess)
ON_MESSAGE(UM_OPEROVERSHOOTPROCESS,OnOpearOvershootProcess)
ON_MESSAGE(UM_OPERCHECKPOLLING,OnOpearCheckPollingProcess)
ON_MESSAGE(UM_OPERSAVERAMDATA,OnOpearSaveRamDataProcess)
ON_BN_CLICKED(IDC_BTN_OPE_READSTART, &COperationDlg::OnBnClickedBtnOpeReadstart)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BTN_OPE_START, &COperationDlg::OnBnClickedBtnOpeStart)
ON_WM_CTLCOLOR()
ON_BN_CLICKED(IDC_BTN_OPE_LEDSET, &COperationDlg::OnBnClickedBtnOpeLedset)
ON_BN_CLICKED(IDC_BTN_OPE_LEDREAD, &COperationDlg::OnBnClickedBtnOpeLedread)
ON_BN_CLICKED(IDC_BTN_OPE_FANON, &COperationDlg::OnBnClickedBtnOpeFanon)
ON_BN_CLICKED(IDC_BTN_OPE_FANOFF, &COperationDlg::OnBnClickedBtnOpeFanoff)
ON_BN_CLICKED(IDC_BTN_OPE_LEDON, &COperationDlg::OnBnClickedBtnOpeLedon)
ON_BN_CLICKED(IDC_BTN_OPE_LENOFF, &COperationDlg::OnBnClickedBtnOpeLenoff)
ON_BN_CLICKED(IDC_BTN_OPE_KPSET, &COperationDlg::OnBnClickedBtnOpeKpset)
ON_BN_CLICKED(IDC_BTN_OPE_KDSET, &COperationDlg::OnBnClickedBtnOpeKdset)
ON_BN_CLICKED(IDC_BTN_OPE_KISET, &COperationDlg::OnBnClickedBtnOpeKiset)
ON_BN_CLICKED(IDC_BTN_OPE_KLSET, &COperationDlg::OnBnClickedBtnOpeKlset)
ON_BN_CLICKED(IDC_BTN_OPE_PIDREAD, &COperationDlg::OnBnClickedBtnOpePidread)
ON_BN_CLICKED(IDC_BTN_OPE_PIDZONESET, &COperationDlg::OnBnClickedBtnOpePidzoneset)
ON_BN_CLICKED(IDC_BTN_OPE_STOP, &COperationDlg::OnBnClickedBtnOpeStop)
ON_BN_CLICKED(IDC_BTN_OPE_FANCLOSETEMP, &COperationDlg::OnBnClickedBtnOpeFanclosetemp)
ON_BN_CLICKED(IDC_BTN_OPE_FANREAD, &COperationDlg::OnBnClickedBtnOpeFanread)
ON_BN_CLICKED(IDC_BTN_OPE_OVERSEND, &COperationDlg::OnBnClickedBtnOpeOversend)
ON_BN_CLICKED(IDC_BTN_OPE_OVERREAD, &COperationDlg::OnBnClickedBtnOpeOverread)
ON_BN_CLICKED(IDC_BTN_OPE_LED2ON, &COperationDlg::OnBnClickedBtnOpeLed2on)
ON_BN_CLICKED(IDC_BTN_OPE_LED2OFF, &COperationDlg::OnBnClickedBtnOpeLed2off)
ON_BN_CLICKED(IDC_BTN_OPE_LED3ON, &COperationDlg::OnBnClickedBtnOpeLed3on)
ON_BN_CLICKED(IDC_BTN_OPE_LED3OFF, &COperationDlg::OnBnClickedBtnOpeLed3off)
ON_BN_CLICKED(IDC_BTN_OPE_LED4ON, &COperationDlg::OnBnClickedBtnOpeLed4on)
ON_BN_CLICKED(IDC_BTN_OPE_LED4OFF, &COperationDlg::OnBnClickedBtnOpeLed4off)
ON_BN_CLICKED(IDC_BTN_OPE_TE_SAVEDATA, &COperationDlg::OnBnClickedBtnOpeTeSavedata)
ON_BN_CLICKED(IDC_BUTTON_SAVJSON, &COperationDlg::OnBnClickedButtonSavjson)
ON_BN_CLICKED(IDC_BUTTON_LDJSON, &COperationDlg::OnBnClickedButtonLdjson)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTEM2, &COperationDlg::OnEnKillfocusEditOpeAnntem2)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTIME2, &COperationDlg::OnEnKillfocusEditOpeAnntime2)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENINGRAMP, &COperationDlg::OnEnKillfocusEditOpeDeningramp)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNRAMP, &COperationDlg::OnEnKillfocusEditOpeAnnramp)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNRAMP2, &COperationDlg::OnEnKillfocusEditOpeAnnramp2)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_EXTRAMP, &COperationDlg::OnEnKillfocusEditOpeExtramp)
ON_BN_CLICKED(IDC_CHECK_ANN2, &COperationDlg::OnBnClickedCheckAnn2)
ON_BN_CLICKED(IDC_CHECK_ENEXT, &COperationDlg::OnBnClickedCheckEnext)
ON_BN_CLICKED(IDC_CHECK_MAXRAMP, &COperationDlg::OnBnClickedCheckMaxramp)
ON_BN_CLICKED(IDC_BUTTON1, &COperationDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON3, &COperationDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//调用主对话框对应消息处理函数
void COperationDlg::OperCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_OperDlg_event,a,b);
}


void COperationDlg::OperReadStatus()
{
//	if (OperReadstautsFlag == 6)	// 所有状态读取完毕
//	{
//		OperReadstautsFlag = 0;		// 读取序号清零			// Zhimin change
//	}
//	else
//	{
		switch(OperReadstautsFlag)
		{
		case 2:		// 读取PI heat温度
			{
				OperDlgFlag = OPEREADSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x10;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x02;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
				TxData[5] = 0x00;		//预留位
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 3:		// 读取风扇状态
			{
				OperDlgFlag = OPEREADSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x10;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x0A;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
				TxData[5] = 0x00;		//预留位
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 4:		// 读取当前循环数和阶段数
			{
				OperDlgFlag = OPEREADSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x14;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x01;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
				TxData[5] = 0x00;		//预留位
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 5:		// 读取当前通道工作状态
			{
				OperDlgFlag = OPEREADSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x14;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x02;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
				TxData[5] = 0x00;		//预留位
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		default:
			break;
		}
//	}
}

// This is a chained read, OperReadstatusFlag cycles from 0 to 6, reading Peltier, PI, PWM etc. with each timer interrupt.

LRESULT COperationDlg::OnOpearProcess(WPARAM wParam, LPARAM lParam)
{
	if (OperReadstautsFlag !=0 )
	{
		CTime recordTime;		
		// add received data decoder code here
		switch(OperReadstautsFlag)
		{
		case 1:		// decoder peltier temperature
			{
				temReadTimerFlag = false;		//一次温度及状态读取内停止timer继续读取

				// float处理
				CString sTem;
				unsigned char cTem[] = {RxData[5],RxData[6],RxData[7],RxData[8]};
				float * fTem = (float *)cTem;
				sTem.Format("%g",*fTem);
				if ( cntReadstatuspel == 1000)		// 显示满1000行后编辑框清零
				{
					//编辑框内容清零
					m_operdlg_sEditpel.Empty();
					SetDlgItemTextA(IDC_EDIT_OPE_PEL,m_operdlg_sEditpel);	// edit display
					cntReadstatuspel = 0;

					//编辑框内重新填写数据
					m_operdlg_sEditpel += (sTem + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PEL,m_operdlg_sEditpel);	// edit display
				}
				else
				{
					m_operdlg_sEditpel += (sTem + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PEL,m_operdlg_sEditpel);	// edit display
				}

				//编辑框垂直滚动到底端
				POINT ptPel;
				GetDlgItem(IDC_EDIT_OPE_PEL)->GetScrollRange(SB_VERT,(LPINT)&ptPel.x,(LPINT)&ptPel.y);
				ptPel.x=0;
				GetDlgItem(IDC_EDIT_OPE_PEL)->SendMessage(EM_LINESCROLL,ptPel.x,ptPel.y);

				CString sPeldata;
				sPeldata.Empty();
				//Get current time
				recordTime = CTime::GetCurrentTime();
				sPeldata.Format("Current time is : %.2dh-%.2dm-%.2ds      ",
					recordTime.GetHour(),recordTime.GetMinute(),recordTime.GetSecond());
				sPeldata += (sTem + "\r\n");
				// receive data save file
				m_readPELFile.Write(sPeldata,sPeldata.GetLength());

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				cntReadstatuspel++;
//				OperReadstautsFlag += 1;
//				OperReadStatus();	// start next read

				break;
			}
		case 2:		// decoder PI heater
			{
				// float处理
				CString sTem;
				unsigned char cTem[] = {RxData[5],RxData[6],RxData[7],RxData[8]};
				float * fTem = (float *)cTem;
				sTem.Format("%g",*fTem);

				if ( cntReadstatuspi == 1000)		// 显示满1000行后编辑框清零
				{
					//编辑框内容清零
					m_operdlg_sEditpiread.Empty();
					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_operdlg_sEditpiread);		// edit display
					cntReadstatuspi = 0;

					//编辑框内重新填写数据
					m_operdlg_sEditpiread += (sTem + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_operdlg_sEditpiread);		// edit display
				}
				else
				{
					m_operdlg_sEditpiread += (sTem + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_operdlg_sEditpiread);		// edit display
				}

				//编辑框垂直滚动到底端
				POINT ptPi;
				GetDlgItem(IDC_EDIT_OPE_PIREAD)->GetScrollRange(SB_VERT,(LPINT)&ptPi.x,(LPINT)&ptPi.y);
				ptPi.x=0;
				GetDlgItem(IDC_EDIT_OPE_PIREAD)->SendMessage(EM_LINESCROLL,ptPi.x,ptPi.y);

				CString sPidata;
				sPidata.Empty();
				//Get current time
				recordTime = CTime::GetCurrentTime();
				sPidata.Format("Current time is : %.2dh-%.2dm-%.2ds      ",
					recordTime.GetHour(),recordTime.GetMinute(),recordTime.GetSecond());
				sPidata += (sTem + "\r\n");
				// receive data save file
				m_readPIFile.Write(sPidata,sPidata.GetLength());

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				cntReadstatuspi++;
//				OperReadstautsFlag += 1;
//				OperReadStatus();	// start next read
				break;
			}
		case 3:		// 读取风扇状态
			{
				CString sTem;
				if (RxData[5] == 0x00)
				sTem = "Fan Status: OFF";
				else
				{
					if (RxData [5] == 0x01)
					//	m_operdlg_sEditvanstatus = "ON";
						sTem = "Fan Status: ON";
					else
					//	m_operdlg_sEditvanstatus = "ERROR";
						sTem = "Fan Status: ERROR";
				}

				SetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sTem);		// edit display

				// receive data save file
				sTem += "     ";
				m_readPELFile.Write(sTem,sTem.GetLength());
				m_readPIFile.Write(sTem,sTem.GetLength());
				
//				OperReadstautsFlag += 1;
				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));
				
//				OperReadStatus();	// start next read
				break;
			}
		case 4:		// 读取cycle状态
			{
				CString stem;
				stem.Format("%d",(RxData[6]+1));			// 当前第几个循环
				m_operdlg_sEditcycleread = stem + " / ";
				stem.Empty();
				stem.Format("%d",RxData[5]);			// 当前第几个阶段
				m_operdlg_sEditcycleread += stem;

				SetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,m_operdlg_sEditcycleread);		// edit display

				//receive data save file
				CString stem1;
				stem1.Empty();
				stem1.Format("Current cycle number: %d / Current stage number: %d     ", RxData[6] + 1, RxData[5]);
				m_readPELFile.Write(stem1,stem1.GetLength());
				m_readPIFile.Write(stem1,stem1.GetLength());
				
				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

//				OperReadstautsFlag += 1;
//				OperReadStatus();	// start next read
				break;
			}
		case 5:		// 读取cycle、pwm、current状态
			{
				BYTE bpwmPel,bpwmPi,bCur;
				bCur = ((RxData[6] & 0x08)>>3);
				bpwmPel = ((RxData[6] & 0x70)>>4);
				bpwmPi = (RxData[6] & 0x07);

				CString sPwmPel, sPwmPi, sCur, stem;
				sPwmPel.Empty();
				sPwmPi.Empty();
				sCur.Empty();
				stem.Empty();
				
				// display current status
				if (bCur == 0)
				{
					SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS, "positive");
					sCur = "Current status is: positive; ";
				}
				else
				{
					SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,"negative");
					sCur = "Current status is: negative; ";
				}

				// display peliter pwm status
				switch(bpwmPel)
				{
				case 0x0:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Idle");
						sPwmPel = "Peltier status is: idle; " ;
						break;
					}
				case 0x1:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Start");
						sPwmPel = "Peltier status is: start; " ;
						break;
					}
				case 0x2:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Adjust");
						sPwmPel = "Peltier status is: adjust; "; 
						break;
					}
				case 0x3:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Count Start");
						sPwmPel = "Peltier status is: count start; "; 
						break;
					}
				case 0x4:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Count End");
						sPwmPel = "Peltier status is: count end; " ;
						break;
					}
				default:
					break;
				}

				// display PI pwm status
				switch(bpwmPi)
				{
				case 0x0:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Idle");
						sPwmPi = "PI status is: idle; " ;
						break;
					}
				case 0x01:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Start");
						sPwmPi = "PI status is: start; " ;
						break;
					}
				case 0x02:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Adjust");
						sPwmPi = "PI status is: adjust; " ;
						break;
					}
				case 0x03:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Count Start");
						sPwmPi = "PI status is: count start; " ;
						break;
					}
				case 0x04:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Count End");
						sPwmPi = "PI status is: count end; " ;
						break;
					}
				default:
					break;
				}

				// read pwm status
				int iPWM0, iPWM1;
				CString sPWM0, sPWM1;
				iPWM0 = 0;
				iPWM1 = 0;
				sPWM0.Empty();
				sPWM1.Empty();

				iPWM0 = (RxData[7]<<8) | RxData[8];
				iPWM1 = (RxData[9]<<8) | RxData[10];
				sPWM0.Format("%d",iPWM0);
				sPWM1.Format("%d",iPWM1);

				if ( cntReadstatuspwm == 1000)		// 显示满1000行后编辑框清零
				{
					//编辑框内容清零
					m_opedlg_sEditpwm1.Empty();
					
					SetDlgItemTextA(IDC_EDIT_OPE_PWM1,m_opedlg_sEditpwm1);		// edit display
					cntReadstatuspwm = 0;

					//编辑框内重新填写数据
					m_opedlg_sEditpwm1 += (sPWM0 + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_opedlg_sEditpwm1);		// edit display

					m_opedlg_sEditpwm2.Empty();
					SetDlgItemTextA(IDC_EDIT_OPE_PWM3,m_opedlg_sEditpwm2);
					m_opedlg_sEditpwm2 += (sPWM1 + "\r\n");
					SetDlgItemTextA(IDC_EDIT_OPE_PWM3,m_opedlg_sEditpwm2);
				}
				else
				{
					m_opedlg_sEditpwm1 += (sPWM0 + "\r\n");		// 取出接收的PWM0数据
					SetDlgItemTextA(IDC_EDIT_OPE_PWM1,m_opedlg_sEditpwm1);		// edit display

					m_opedlg_sEditpwm2 += (sPWM1 + "\r\n");		// 取出接收的PWM1数据
					SetDlgItemTextA(IDC_EDIT_OPE_PWM3,m_opedlg_sEditpwm2);		// edit display
				}

				//编辑框垂直滚动到底端
				POINT ptPi;
				GetDlgItem(IDC_EDIT_OPE_PWM1)->GetScrollRange(SB_VERT,(LPINT)&ptPi.x,(LPINT)&ptPi.y);
				GetDlgItem(IDC_EDIT_OPE_PWM3)->GetScrollRange(SB_VERT,(LPINT)&ptPi.x,(LPINT)&ptPi.y);
				ptPi.x=0;
				GetDlgItem(IDC_EDIT_OPE_PWM1)->SendMessage(EM_LINESCROLL,ptPi.x,ptPi.y);
				GetDlgItem(IDC_EDIT_OPE_PWM3)->SendMessage(EM_LINESCROLL,ptPi.x,ptPi.y);

				// receive data save file
				stem =sCur + sPwmPel + sPwmPi + " Sensor0 PWM is: "+ sPWM0 + " Sensor0 PWM is: "+sPWM1 +"\r\n";
				m_readPELFile.Write(stem,stem.GetLength());
				m_readPIFile.Write(stem,stem.GetLength());

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

//				OperReadstautsFlag += 1;
//				OperReadStatus();	// start next read

				temReadTimerFlag = true;		//一次温度及状态读取完毕允许timer进行下次读取
				break;
			}
		default:
			break;
		}
	}

	return 0;
}

LRESULT COperationDlg::OnOpearPIDProcess(WPARAM wParam, LPARAM lParam)
{
	CString sKP0,sKP1,sKI0,sKI1,sKD0,sKD1,sKL0,sKL1,sTemp;
	sKP0.Empty();
	sKP1.Empty();
	sKI0.Empty();
	sKI1.Empty();
	sKD0.Empty();
	sKD1.Empty();
	sKL0.Empty();
	sKL1.Empty();
	sTemp.Empty();
	
	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
//	float * fTem = (float *)cTem;
//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;
	
	// get tempture
	cTem[0] = RxData[6];
	cTem[1] = RxData[7];
	cTem[2] = RxData[8];
	cTem[3] = RxData[9];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sTemp = sTem;

	// get kp0
	sTem.Empty();
	cTem[0] = RxData[10];
	cTem[1] = RxData[11];
	cTem[2] = RxData[12];
	cTem[3] = RxData[13];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKP0 = sTem;

	// get ki0
	sTem.Empty();
	cTem[0] = RxData[14];
	cTem[1] = RxData[15];
	cTem[2] = RxData[16];
	cTem[3] = RxData[17];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKI0 = sTem;

	// get kd0
	sTem.Empty();
	cTem[0] = RxData[18];
	cTem[1] = RxData[19];
	cTem[2] = RxData[20];
	cTem[3] = RxData[21];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKD0 = sTem;

	// get kl0
	sTem.Empty();
	cTem[0] = RxData[22];
	cTem[1] = RxData[23];
	cTem[2] = RxData[24];
	cTem[3] = RxData[25];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKL0 = sTem;

	// get kp1
	sTem.Empty();
	cTem[0] = RxData[26];
	cTem[1] = RxData[27];
	cTem[2] = RxData[28];
	cTem[3] = RxData[29];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKP1 = sTem;

	// get ki1
	sTem.Empty();
	cTem[0] = RxData[30];
	cTem[1] = RxData[31];
	cTem[2] = RxData[32];
	cTem[3] = RxData[33];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKI1 = sTem;

	// get kd1
	sTem.Empty();
	cTem[0] = RxData[34];
	cTem[1] = RxData[35];
	cTem[2] = RxData[36];
	cTem[3] = RxData[37];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKD1 = sTem;

	// get kl1
	sTem.Empty();
	cTem[0] = RxData[38];
	cTem[1] = RxData[39];
	cTem[2] = RxData[40];
	cTem[3] = RxData[41];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKL1 = sTem;

	SetDlgItemTextA(IDC_EDIT_OPE_KP0,sKP0);
	SetDlgItemTextA(IDC_EDIT_OPE_KI0,sKI0);
	SetDlgItemTextA(IDC_EDIT_OPE_KD0,sKD0);
	SetDlgItemTextA(IDC_EDIT_OPE_KL0,sKL0);
	SetDlgItemTextA(IDC_EDIT_OPE_KP1,sKP1);
	SetDlgItemTextA(IDC_EDIT_OPE_KI1,sKI1);
	SetDlgItemTextA(IDC_EDIT_OPE_KD1,sKD1);
	SetDlgItemTextA(IDC_EDIT_OPE_KL1,sKL1);
	SetDlgItemTextA(IDC_EDIT_OPE_PIDZONE,sTemp);

	//清除数据传输buffer
	memset(RxData,0,sizeof(RxData));

	return 0;
}


LRESULT COperationDlg::OnOpearLEDProcess(WPARAM wParam, LPARAM lParam)
{
	CString sSetup,sHold;
	sSetup.Empty();
	sHold.Empty();

	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
	//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
	//	float * fTem = (float *)cTem;
	//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;

	switch(RxData[4])
	{
	case 0x21:		// read led setup time
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sSetup = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_LEDSETTIMEREAD,sSetup);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x22:		// read led hold time
		{
			// get hold time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sHold = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_LEDHOLDTIMEREAD,sHold);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x05:		// read fan pre-close temperature
		{
			// get the temperature
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sHold = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_FANREAD,sHold);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));
		}
	default:	break;
	}
	return 0;
}


LRESULT COperationDlg::OnOpearOvershootProcess(WPARAM wParam, LPARAM lParam)
{
	CString sTime,sTemp;
	sTime.Empty();
	sTemp.Empty();

	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
	//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
	//	float * fTem = (float *)cTem;
	//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;

	switch(RxData[4])
	{
	case 0x05:		// read overshoot time
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTime = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_OVERREADTIME,sTime);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x06:		// read overshoot temp
		{
			// get hold time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTemp = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_OVERREADTEMP,sTemp);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	default:	break;
	}
	return 0;
}


LRESULT COperationDlg::OnOpearCheckPollingProcess(WPARAM wParam, LPARAM lParam)
{
	OperDlgFlag = OPEREADSTATUS;
	
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x14;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x15;		//data type
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	return 0;
}

void COperationDlg::ResetParam()
{
	if (ReceiveTemCycNum == 0) {		// cycler not running
		// To do: Send command to reset cycler parameters
		curPiTemp = 105;
		curInitDenTemp = 95;
		curInitDenTime = 120;
		curDenTemp = 95;
		curDenTime = 15;
		curDenRamp = 5;
		curAnnTemp = 60;
		curAnnTime = 20;
		curAnnRamp = 2;
		curExtTemp = 60;
		curExtTime = 20;
		curExtRamp = 2;

		curHoldTemp = 50;
		curHoldTime = 10;
		curANNCycle = 50;
	}
	else {
		// Do nothing, assuming it is a USB glitch. 
		// Maybe I should report such glitch.
	}
}

BOOL COperationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	// initial edit back-color brush
	m_editBrush.CreateSolidBrush(RGB(255,250,0));

	// Set parameter default
	sPITem_write = PITEMP;
	sCycle = ANNCYCLE;
	sInidenTem_write = INIDENTEMP;
	sInidenTim_write = INIDENTIME;
	sDenTem_write = DENTEMP;
	sDenTim_write = DENTIME;
	sDenRam_write = DENRAMP;
	sAnnTem_write = ANNTEMP;
	sAnnTim_write = ANNTIME;
	sAnnRam_write = ANNRAMP;
	sAnnTem2_write = ANNTEMP2;
	sAnnTim2_write = ANNTIME2;
	sAnnRam2_write = ANNRAMP2;
	sExtTem_write = EXTTEMP;
	sExtTim_write = EXTTIME;
	sExtRam_write = EXTRAMP;
	sHoldTem_write = HOLDTEMP;
	sHoldTim_write = HOLDTIME;

	SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGRAMP, sDenRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP, sAnnRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM2, sAnnTem2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME2, sAnnTim2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP2, sAnnRam2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTRAMP, sExtRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);

	((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERTIME))->SetWindowText("5");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERTEMP))->SetWindowText("4");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERREADTIME))->SetWindowText("5");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERREADTEMP))->SetWindowText("10");			// changed according to Xiao Hong request 

	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KP0))->SetWindowText("50");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KP1))->SetWindowText("200");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KI0))->SetWindowText("0.05");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KI1))->SetWindowText("0.15");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KD0))->SetWindowText("20");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KD1))->SetWindowText("20");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KL0))->SetWindowText("0");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_KL1))->SetWindowText("0");
	((CEdit *)GetDlgItem(IDC_EDIT_OPE_PIDZONE))->SetWindowText("80");

	// ((CButton*)GetDlgItem(IDC_CHECK_OPE_MASK4))->EnableWindow(false);

	InitSimTempChart();	// init chart

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


// COperationDlg message handlers


BOOL COperationDlg::PreTranslateMessage(MSG* pMsg)
{

if ((WM_KEYFIRST <= pMsg->message) && (pMsg->message <= WM_KEYLAST))
{
	if (pMsg->wParam == VK_RETURN)
	{
		::SetFocus(m_hWnd);
		return TRUE;
	}
}
return FALSE;
}


void COperationDlg::OnEnKillfocusEditOpePitem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_PITEM, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
		SetDlgItemTextA(IDC_EDIT_OPE_PITEM, sPITem_write);
	}
	else {
		sPITem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
	}

//	if (MessageBox("Accept input value "+sTemp+"?","Input value",MB_YESNO) == IDYES)

//	else
//		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
}


void COperationDlg::OnEnKillfocusEditOpeDentem()
{
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENTEM,sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sInidenTem_write = sTemp;
	}

	SetDlgItemTextA(IDC_EDIT_OPE_DENTEM, sInidenTem_write);
}

void COperationDlg::OnEnKillfocusEditOpeDentime()
{
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 1200");
	}
	else {
		sInidenTim_write = sTemp;
	}

	SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeDeningtem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM,sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sDenTem_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeDeningtime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 1200");
	}
	else {
		sDenTim_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnntem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTEM,sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sAnnTem_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnntime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 1200");
	}
	else {
		sAnnTim_write = sTemp;	
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnncycle()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 100) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 100");
	}
	else {
		sCycle = sTemp;
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
}


void COperationDlg::OnEnKillfocusEditOpeExttem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_EXTTEM,sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sExtTem_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeExttime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_EXTTIME,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 1200");
	}
	else {
		sExtTim_write = sTemp;	
		UpdateSimTempCurve();
	}
		
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeHoldtem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sHoldTem_write = sTemp;
	}

	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeHoldtime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 0)) {
		MessageBox("Please set value between 0 and 1200");
	}
	else {
		sHoldTim_write = sTemp;
	}

	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeKp()
{
	// TODO: Add your control notification handler code here
/*	
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KP,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKP = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KP,sKP);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KP,sKP);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKi()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KI,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKI = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KI,sKI);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KI,sKI);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKd()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KD,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKD = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KD,sKD);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KD,sKD);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKl()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KL,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKL = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KL,sKL);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KL,sKL);
*/
}


void COperationDlg::OnBnClickedBtnOpeSavestatus()
{
	// TODO: Add your control notification handler code here

	CString sTemp,sTemp2,sStatus;
	sTemp.Empty();
	sTemp2.Empty();
	sStatus.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PEL,sTemp);
	sStatus += (sTemp2 + "Peltier Data: " + "\r\n" + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PIREAD,sTemp);
	sStatus += (sTemp2 + "PI Data: " + "\r\n" + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,sTemp);
	sStatus += ("Cycle is: " + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,sTemp);
	sStatus += ("Peltier PWM is: " + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PWMPI,sTemp);
	sStatus += ("PI PWM is: " + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sTemp);
	sStatus += ("Van Status is: " + sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,sTemp);
	sStatus += ("Current Status is: " + sTemp + "\r\n");

	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"文本文件(*.txt)|*.txt|""文档文件(*.doc)|*.doc|""表格文件(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		CFile nFile(saveDlg.GetPathName(),CFile::modeCreate | CFile::modeReadWrite);   // 创建文件，用选取的文件名
		nFile.Write(sStatus,sStatus.GetLength());		// 将要保存的数据写入文件 （CString sStatus为要写入的内容）
		nFile.Close();		// 完成文件写入
	}
}


void COperationDlg::OnBnClickedBtnOpeClear()
{
	// TODO: Add your control notification handler code here
	CString sEmpty;
	sEmpty.Empty();
	SetDlgItemTextA(IDC_EDIT_OPE_PEL,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,sEmpty);
	m_operdlg_sEditpel.Empty();
	m_operdlg_sEditpiread.Empty();
}


void COperationDlg::OnBnClickedBtnOpeSavepara()
{
	// TODO: Add your control notification handler code here

	// Get current parameter setting

	// 显示保存窗口
	m_ParsaveDlg.Create(IDD_PARSAVE_DIALOG,this);
	m_ParsaveDlg.ShowWindow(1);

	// 获取参数值
	CString sTemp;
	GetDlgItemTextA(IDC_EDIT_OPE_PITEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");

}


//int StartPos = 0;	
//int EndPos = 0;		
//int cNum = 0;		
//int enterkeyNum = 0;	

// Load cycler parameter from a text file - obsolete

void COperationDlg::OnBnClickedBtnOpeLoad()
{
	// TODO: Add your control notification handler code here
	
	CString sFilePath;
	CFileDialog FileOpenDialog(TRUE);
	FileOpenDialog.m_ofn.Flags = OFN_ENABLEHOOK|OFN_EXPLORER;
	CString strFilter;
	strFilter = _T("TXT");
	strFilter += (TCHAR)'\0'; 
	strFilter += _T("*.txt");
	strFilter += (TCHAR)'\0'; 

	strFilter += _T("DAT");
	strFilter += (TCHAR)'\0';
	strFilter += _T("*.dat");
	strFilter += (TCHAR)'\0';

	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';  
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0'; 

	FileOpenDialog.m_ofn.lpstrFilter = strFilter;
	FileOpenDialog.m_ofn.lpstrTitle = "Download Filename...";
	CString sFileName;
	sFileName.Empty();
	if ((((CMDIFrameWnd *) AfxGetMainWnd())->MDIGetActive()) != NULL)
		sFileName = (((CMDIFrameWnd *) AfxGetMainWnd())->MDIGetActive())->GetActiveDocument()->GetTitle();
	if (!sFileName.IsEmpty())
	{
		if (sFileName.Find('.') != -1)
			sFileName = sFileName.Left(sFileName.Find('.'));
		sFileName += _T(".hex");
		strcpy(FileOpenDialog.m_ofn.lpstrFile,LPCTSTR(sFileName));
	}
	BOOL bResult = FileOpenDialog.DoModal() == IDOK ? TRUE : FALSE;
	if (bResult)
	{
		sFilePath = FileOpenDialog.GetPathName();	
		HANDLE fileHand;
		DWORD fileSize;
		fileHand = CreateFile(sFilePath,0,0,0,OPEN_EXISTING,0,0);
		fileSize = GetFileSize(fileHand,0);
		char * bufFile = new char [fileSize];
		memset(bufFile,0,fileSize);
		CFile file;
		file.Open(sFilePath,CFile::modeRead);	
		file.Read(bufFile,fileSize);
		CString sFile;
		sFile = bufFile;
		delete [] bufFile;
//		CString sFind = "\r\n";
		CString sFind = " ";
		int StartPos = 0;	
		int EndPos = 0;		
		int cNum = 0;		
		int enterkeyNum = 0;	
		while((EndPos=sFile.Find(sFind,StartPos)) != -1)
		{
			StartPos = EndPos+1;
			enterkeyNum ++;
		}
		CString* sEdit = new CString [enterkeyNum];	
		for(int i=0; i<enterkeyNum; i++)
			sEdit[i].Empty();
		StartPos = 0;
		EndPos = 0;
		if (enterkeyNum != 13)		
		{
			AfxMessageBox("The number of parameter is wrong");
		}
		else
		{
			for (int j=0; j<enterkeyNum; j++)
			{
				EndPos = sFile.Find(sFind,StartPos);
				cNum = EndPos - StartPos;
				unsigned char * cEdit = new unsigned char[cNum+1];
				memset(cEdit,0,cNum+1);
				for (int i=0; i<=cNum; i++)
				{
//					cEdit[i] = sFile[(StartPos+i)];
					if(i == cNum)
						cEdit[i] = '\0';
					else
						cEdit[i] = sFile.GetAt(StartPos+i);				
				}
//				sEdit[j] = static_cast(cEdit);
//				sEdit[j].Format("%s",cEdit);
				sEdit[j] = cEdit;
				StartPos = EndPos+1;
				delete[] cEdit;
			}			
		}

		sPITem_write = sEdit[1];
		sInidenTem_write = sEdit[2];
		sInidenTim_write = sEdit[3];
		sDenTem_write = sEdit[4];
		sDenTim_write = sEdit[5];
		sAnnTem_write = sEdit[6];
		sAnnTim_write = sEdit[7];
		sCycle = sEdit[8];
		sExtTem_write = sEdit[9];
		sExtTim_write = sEdit[10];
		sHoldTem_write = sEdit[11];
		sHoldTim_write = sEdit[12];

		delete []sEdit;

		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
	}
	else
		AfxMessageBox("Can't find the file");	
}


void COperationDlg::OnBnClickedBtnOpeMore()
{
	// TODO: Add your control notification handler code here
//	m_ManuDlg.Create(IDD_MANUALCTR_DIALOG,this);
//	m_ManuDlg.ShowWindow(1);
	CDialog * pDlg = new CDialog;
	pDlg->Create(IDD_MANUALCTR_DIALOG);
	pDlg->ShowWindow(1);
}

// Start/stop recording temperature

void COperationDlg::OnBnClickedBtnOpeReadstart()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	if (ReadTemBtnStatus)
	{
		SetDlgItemTextA(IDC_BTN_OPE_READSTART,"Stop");
		ReadTemBtnStatus = ! ReadTemBtnStatus;
	}
	else
	{
		SetDlgItemTextA(IDC_BTN_OPE_READSTART,"Start");
		ReadTemBtnStatus = ! ReadTemBtnStatus;
	}
	
	CString sPELfilename,sPIfilename;
	sPELfilename.Empty();
	sPIfilename.Empty();

	if (opeDlgTimerFlag == 0 )	// timer 未打开
	{
		opeDlgTimerFlag = 1;
		temReadTimerFlag = true;	//允许timer工作
		SetTimer(1,RTEMTIMERVALUE,NULL);	//设置连续发送的timer1
		sysTime = CTime::GetCurrentTime();
		sTime.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
					sysTime.GetYear(),sysTime.GetMonth(),sysTime.GetDay(),
					sysTime.GetHour(),sysTime.GetMinute(),sysTime.GetSecond());
		sPELfilename = "PELTemp_" + sTime + ".txt";
		sPIfilename = "PITemp_" + sTime + ".txt";
		m_readPIFile.Open(sPIfilename,CFile::modeCreate|CFile::modeWrite);
		m_readPELFile.Open(sPELfilename,CFile::modeCreate|CFile::modeWrite);
	}		
	else
	{
		opeDlgTimerFlag = 0;
		KillTimer(1);

		m_readPELFile.Close();
		m_readPIFile.Close();
	}				
}

// Command 10 02, read temperature

void COperationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch (nIDEvent)
	{
	case 1:
		{
//			if ( timerCtrFlag & temReadTimerFlag &(!PollingGraTimerFlag) )

	// Zhimin Ding added:

//		CString str;
//		str.Empty();
//		str.Format("%d", ReceiveTemCycNum);
//		SetDlgItemTextA(IDC_EDIT_OPE_RCVTEMCYCNUM, str);

	//====================

			if ( timerCtrFlag & temReadTimerFlag  )
			{
				// 读取peltier温度
/*				OperReadstautsFlag = 1;
				OperDlgFlag = OPEREADSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x10;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x02;		//data type, date edit first byte
				TxData[4] = 0x02;		//real data
				TxData[5] = 0x00;		//预留位
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
*/
				OperReadStatus();

				if (OperReadstautsFlag == 6)
					OperReadstautsFlag = 3;
				else
					OperReadstautsFlag++;

			}
			break;
		}
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

// Moved to trim dialog.
/*
void COperationDlg::OperGetPCRMaskStatus()
{
	if ((BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_OPE_MASK1)))
		ck1 = 1;
	else
		ck1 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_OPE_MASK2))
		ck2 = 1;
	else
		ck2 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_OPE_MASK3))
		ck3 = 1;
	else
		ck3 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_OPE_MASK4))
		ck4 = 1;
	else
		ck4 = 0;

	PCRMask = (ck4<<3) | (ck3<<2) | (ck2<<1) | ck1;

	ck1 = 0;
	ck2 = 0;
	ck3 = 0;
	ck4 = 0;

}
*/

extern CTrimDlg *g_pTrimDlg;
extern BOOL g_Melt;

void COperationDlg::OnBnClickedBtnOpeStart()
{
	// TODO: Add your control notification handler code here

/*	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}
*/

	AutoSampleFlag = TRUE;			// This allows graphic polling

	// variable initial
	CString stime, stempture, sramp;
	float ftempture = 0, framp = 1;
	int itime = 0;
	
	stime.Empty();
	stempture.Empty();

	TemperCycNum = 0;

	ASSERT(ReceiveTemCycNum == 0);

//	if (ReceiveTemCycNum > 0) {
//		MessageBox("Not ready yet! (ReceiveTemCycNum > 0)");
//		return;
//	}

	//reset picture data, by ych - This is now done in GraDlg
/*	for (int ii = 0; ii < 4; ii++) {
		for (int ij = 0; ij < numWells; ij++) {
			m_yData[ii][ij].clear();
		}
	}
*/
	//if (g_pTrimDlg)						// Called within PCRProjDlg instead.
		//g_pTrimDlg->CommitTrim();

	// OperGetPCRMaskStatus();

	OperDlgFlag = OPEGETPCRMASK;
	OperCalMainMsg();

	// first send
	// send graphic HID mask for trigger
	SetPCRMask(PCRMask);
	Sleep(PIDDELAY);

	OnBnClickedBtnOpeOversend();				// Zhimin added: insert a send here if the overshoot params have changed. But not commited.

	//////////////////////
	// Melt Curve

	if (g_Melt) {
		SetMeltCurve();
		Sleep(PIDDELAY);
		return;
	}


	/////////////////////

	//	GetDlgItemText(IDC_EDIT_OPE_PITEM,stempture);		//取PI heater编辑框中的数据
	//	ftempture = (float)atof(stempture);	//将tempture编辑框的字符串转成浮点型变量

	// second send - Pi Temp or hot lid temp
	float epsilon = 0.01;

	ftempture = (float)atof(sPITem_write);
	if (1 || abs(curPiTemp - ftempture) > epsilon) {
		curPiTemp = ftempture;
		SetPiTemp(curPiTemp);

		Sleep(PIDDELAY);
	}

	// third send: set temp cycle params
	BOOL commit = false;

	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM,stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME,stime);
	GetDlgItemText(IDC_EDIT_OPE_DENINGRAMP, sramp);
	ftempture = (float)atof(stempture);		//
	itime = atoi(stime);					//
	framp = (float)atof(sramp);

	if (abs(ftempture - curDenTemp) > epsilon) {
		curDenTemp = ftempture;
		commit = true;
	}
	if(curDenTime != itime) {
		curDenTime = itime;
		commit = true;
	}
	if (abs(framp - curDenRamp) > epsilon) {
		curDenRamp = framp;
		commit = true;
	}

	GetDlgItemText(IDC_EDIT_OPE_ANNTEM,stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME,stime);
	GetDlgItemText(IDC_EDIT_OPE_ANNRAMP, sramp);
	ftempture = (float)atof(stempture);		//
	itime = atoi(stime);					//
	framp = (float)atof(sramp);

	if (abs(ftempture - curAnnTemp) > epsilon) {
		curAnnTemp = ftempture;
		commit = true;
	}
	if (curAnnTime != itime) {
		curAnnTime = itime;
		commit = true;
	}
	if (abs(framp - curAnnRamp) > epsilon) {
		curAnnRamp = framp;
		commit = true;
	}

	GetDlgItemText(IDC_EDIT_OPE_ANNTEM2, stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME2, stime);
	GetDlgItemText(IDC_EDIT_OPE_ANNRAMP2, sramp);
	ftempture = (float)atof(stempture);		//
	itime = atoi(stime);					//
	framp = (float)atof(sramp);

	if (abs(ftempture - curAnnTemp2) > epsilon) {
		curAnnTemp2 = ftempture;
		commit = true;
	}
	if (curAnnTime2 != itime) {
		curAnnTime2 = itime;
		commit = true;
	}
	if (abs(framp - curAnnRamp2) > epsilon) {
		curAnnRamp2 = framp;
		commit = true;
	}

	GetDlgItemText(IDC_EDIT_OPE_EXTTEM,stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_EXTTIME,stime);
	GetDlgItemText(IDC_EDIT_OPE_EXTRAMP, sramp);

	ftempture = (float)atof(stempture);					//
	itime = atoi(stime);								//
	framp = (float)atof(sramp);

	if (abs(ftempture - curExtTemp) > epsilon) {
		curExtTemp = ftempture;
		commit = true;
	}
	if (curExtTime != itime) {
		curExtTime = itime;
		commit = true;
	}
	if (abs(framp - curExtRamp) > epsilon) {
		curExtRamp = framp;
		commit = true;
	}

//		This is the original "13 03" command
//	if (commit || 1) {
//		SetPCRCyclTempTime(curDenTemp, curDenTime, curAnnTemp, curAnnTime, curExtTemp, curExtTime);
//		Sleep(PIDDELAY);
//	}

// These are the new "13 09" commands that allow ramp rate specification
//	if (commit || 1) {
//		SetPCRCyclTempTimeNew(curDenTemp, curDenTime, curDenRamp, curAnnTemp, curAnnTime, curAnnRamp, curExtTemp, curExtTime, curExtRamp);
//		Sleep(PIDDELAY);
//	}

//	if (commit || 1) {
//		SetPCRCyclTempTime4Seg(curDenTemp, curDenTime, curDenRamp, curAnnTemp, curAnnTime, curAnnRamp, curAnnTemp2, curAnnTime2, curAnnRamp2, curExtTemp, curExtTime, curExtRamp);
//		Sleep(PIDDELAY);
//	}
	if (m_bMaxRamp) {
		if (m_EnableExtension) {
			SetPCRCyclTempTime(curDenTemp, curDenTime, curAnnTemp, curAnnTime, curExtTemp, curExtTime);
			Sleep(PIDDELAY);
		}
		else {
			SetPCRCyclTempTime2Seg(curDenTemp, curDenTime, curAnnTemp, curAnnTime);
			Sleep(PIDDELAY);
		}
	}
	else if (!m_EnableAnnealing2) {
		if (m_EnableExtension) {
			SetPCRCyclTempTimeNew(curDenTemp, curDenTime, curDenRamp, curAnnTemp, curAnnTime, curAnnRamp, curExtTemp, curExtTime, curExtRamp);
			Sleep(PIDDELAY);
		}
		else {
			SetPCRCyclTempTimeNew2Seg(curDenTemp, curDenTime, curDenRamp, curAnnTemp, curAnnTime, curAnnRamp);
			Sleep(PIDDELAY);
		}
	}
	else {
		SetPCRCyclTempTime4Seg(curDenTemp, curDenTime, curDenRamp, curAnnTemp, curAnnTime, curAnnRamp, curAnnTemp2, curAnnTime2, curAnnRamp2, curExtTemp, curExtTime, curExtRamp);
		Sleep(PIDDELAY);
	}

//	Sleep(30000);
//	while(!timerCtrFlag);

	// forth send: init denature, hold and cycle numbers

	commit = false;

	GetDlgItemText(IDC_EDIT_OPE_DENTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,stime);
	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	itime = atoi(stime);					//将编辑框中整型字符串转成byte

	if (abs(ftempture - curInitDenTemp) > epsilon) {
		curInitDenTemp = ftempture;
		commit = true;
	}
	if (curInitDenTime != itime) {
		curInitDenTime = itime;
		commit = true;
	}

	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,stime);
	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	itime = atoi(stime);	//将编辑框中整型字符串转成byte

	if (abs(ftempture - curHoldTemp) > epsilon) {
		curHoldTemp = ftempture;
		commit = true;
	}
	if (curHoldTime != itime) {
		curHoldTime = itime;
		commit = true;
	}

	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,stime);
	itime = atoi(stime);

	TemperCycNum = itime;

	if (curANNCycle != itime) {
		curANNCycle = itime;
		commit = true;
	}

	if (commit || 1) {			// Zhimin comment; we always need to call this to start the cycler  
		SetPCRCyclTempTime2(curInitDenTemp, curInitDenTime, curHoldTemp, curHoldTime, curANNCycle);
		Sleep(PIDDELAY);
	}
}


void COperationDlg::OnBnClickedBtnOpeStop()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	AutoSampleFlag = FALSE;
	ReceiveTemCycNum = 0;

	OperDlgFlag = OPEREADSTATUS;
	
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x00;		//data type				// Zhimin comment: stop the heater
	TxData[4] = 0x02;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// second send
	OperDlgFlag = OPEREADSTATUS;

	CString stime, stempture;
	float ftempture;
	int itime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_DENTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = itime>>8;
	OperBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = itime>>8;
	OperBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,stime);
	itime = atoi(stime);
	OperBuf[12] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x10;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte TXC
	TxData[4] = 0x00;		//real data, start			// Zhimin comment: this is to stop the cycler.
	TxData[5] = OperBuf[12];	//cycle setting
	TxData[6] = 0x00;       //
	TxData[7] = OperBuf[0];	//inital dennature数据	
	TxData[8] = OperBuf[1];  //
	TxData[9] =	OperBuf[2];		
	TxData[10] = OperBuf[3];	
	TxData[11] = OperBuf[4];	
	TxData[12] = OperBuf[5];		
	TxData[13] = OperBuf[6];	//extern extension数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	//TxData[18] = RegBuf[1];
	//TxData[18] = 0x01;
	for (int i=1; i<19; i++)
		TxData[19] += TxData[i];
	if (TxData[19]==0x17)
		TxData[19]=0x18;
	else
		TxData[19]=TxData[19];
	TxData[20]=0x17;
	TxData[21]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


HBRUSH COperationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
/*
	// set peliter & pi read edit back_color
	if (nCtlColor==CTLCOLOR_EDIT && (pWnd->GetDlgCtrlID()== IDC_EDIT_OPE_PEL | pWnd->GetDlgCtrlID()== IDC_EDIT_OPE_PIREAD) )
	{
		pDC -> SetTextColor(RGB(255,0,0));
		pDC -> SetBkMode(TRANSPARENT);
		hbr = m_editBrush;
	}
*/
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void COperationDlg::OnBnClickedBtnOpeLedset()
{
	// TODO: Add your control notification handler code here

	// send led setup time command
	OperDlgFlag = OPEREADSTATUS;

	CString stime;
	float fstime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_LEDSETTIME,stime);		//取LED setup time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x21;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;
	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send led hold time command
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_LEDHOLDTIME,stime);		//取LED hold time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x22;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序


	
}


void COperationDlg::OnBnClickedBtnOpeLedread()
{
	// TODO: Add your control notification handler code here
	
	// read led setup time
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x21;		//data type, read setup time
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// read led hold time
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x22;		//data type, read hold time
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanon()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x03;		//data type, fan control
	TxData[4] = 0x03;		//real data, start to auto fan	
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanoff()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command 
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x03;		//data type, fan control
	TxData[4] = 0x02;		//real data, close to auto fan		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKpset()
{
	// TODO: Add your control notification handler code here
	
	CString sData;
	float fData;
	unsigned char * hData;
	
	// send zone 0 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KP0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x01;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KP1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x11;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKdset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KD0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x04;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KD1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x14;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKiset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KI0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x02;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KI1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x12;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKlset()
{
	// TODO: Add your control notification handler code here
	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KL0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x08;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KL1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x18;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpePidread()
{
	// TODO: Add your control notification handler code here

	// send zone 1 data
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x12;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x01;		//data type
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpePidzoneset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	OperDlgFlag = OPEREADSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_PIDZONE,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x09;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}

void COperationDlg::OnBnClickedBtnOpeFanclosetemp()
{
	// TODO: Add your control notification handler code here
	// send led setup time command
	OperDlgFlag = OPEREADSTATUS;

	CString stime;
	float fstime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_FANCLOSETEMP,stime);		//取LED setup time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x04;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;
	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanread()
{
	// TODO: Add your control notification handler code here

	// send led setup time command
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x05;		//data type
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeOversend()
{
	// send overshoot setup time command

//	if (MessageBox("Send Overshoot Configuration?", "Overshoot", MB_YESNO) != IDYES) return;

	BOOL commit = false;
	float epsilon = 0.01;

	CString		stime;
	float		fstime;
	unsigned char *hData;

	GetDlgItemText(IDC_EDIT_OPE_OVERTIME, stime);		//overshoot set time
	fstime = (float)atof(stime);						//
	hData = (unsigned char *)&fstime;

	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	if (abs(curOverTime - fstime) > epsilon) {
		commit = true;
		curOverTime = fstime;
	}

	// send overshoot temp command

	GetDlgItemText(IDC_EDIT_OPE_OVERTEMP, stime);		//overshoot temp
	fstime = (float)atof(stime);		//
	hData = (unsigned char *)&fstime;
	OperBuf[4] = hData[0];
	OperBuf[5] = hData[1];
	OperBuf[6] = hData[2];
	OperBuf[7] = hData[3];

	if (abs(curOverTemp - fstime) > epsilon) {
		commit = true;
		curOverTemp = fstime;
	}

// ====================================

	GetDlgItemText(IDC_EDIT_OPE_OVERREADTIME, stime);		//overshoot set time
	fstime = (float)atof(stime);						//
	hData = (unsigned char *)&fstime;

	OperBuf[8] = hData[0];
	OperBuf[9] = hData[1];
	OperBuf[10] = hData[2];
	OperBuf[11] = hData[3];

	if (abs(curOverTime2 - fstime) > epsilon) {
		commit = true;
		curOverTime2 = fstime;
	}

	// send overshoot temp command

	GetDlgItemText(IDC_EDIT_OPE_OVERREADTEMP, stime);		//overshoot temp
	fstime = (float)atof(stime);		//
	hData = (unsigned char *)&fstime;
	OperBuf[12] = hData[0];
	OperBuf[13] = hData[1];
	OperBuf[14] = hData[2];
	OperBuf[15] = hData[3];

	if (abs(curOverTemp2 - fstime) > epsilon) {
		commit = true;
		curOverTemp2 = fstime;
	}

	//Send message to main dialog
	if (commit) {
		MakeGraPacket(0x13, 0x8, OperBuf, 16);	//
		Sleep(200);
	}
}

void COperationDlg::OnBnClickedBtnOpeOverread()
{
	// TODO: Add your control notification handler code here

	// read overshoot time
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x14;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x05;		//data type, read setup time
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// read overshoot temp
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x14;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x06;		//data type, read hold time
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}

// maximum 16 bytes of data, first byte reserved
void COperationDlg::MakeGraPacket(byte pCmd, byte pType, byte pData[], int num_data)
{
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = pCmd;		//command
	TxData[2] = 18;			//data length
	TxData[3] = pType;		//data type, date edit first byte

	TxData[4] = 0x0;		//real data, first byte reserved
/*	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = 0x00;
	TxData[16] = 0x00;
	TxData[17] = 0x00;
	TxData[18] = 0x00;
	TxData[19] = 0x00;
	TxData[20] = 0x00;
*/

	for (int i = 0; i < num_data; i++) {
		TxData[5 + i] = pData[i];
	}

	TxData[21] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5] + TxData[6] + TxData[7] + TxData[8] + TxData[9]
		+ TxData[10] + TxData[11] + TxData[12] + TxData[13] + TxData[14] + TxData[15] + TxData[16] + TxData[17] + TxData[18] + TxData[19] + TxData[20];		//check sum
	
	if (TxData[21] == 0x17)
		TxData[21] = 0x18;
//	else
//		TxData[17] = TxData[17];

	TxData[22] = 0x17;		//back code
	TxData[23] = 0x17;		//back code

							//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}

void COperationDlg::OnBnClickedBtnOpeLedon()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x81;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLenoff()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x80;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed2on()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x82;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed2off()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x80;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed3on()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x84;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed3off()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x80;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed4on()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x88;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLed4off()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = OPEREADSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x80;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeTeSavedata()
{
	// TODO: Add your control notification handler code here

	CString sTeDataFileName;
	sTeDataFileName.Empty();

	if(saveTeDataCnt == 0)
	{
		m_saveTeDataFile.Close();

		sysTime = CTime::GetCurrentTime();
		sTime.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
			sysTime.GetYear(),sysTime.GetMonth(),sysTime.GetDay(),
			sysTime.GetHour(),sysTime.GetMinute(),sysTime.GetSecond());
		sTeDataFileName = "RamData_" + sTime + ".txt";

		m_saveTeDataFile.Open(sTeDataFileName,CFile::modeCreate|CFile::modeWrite);
	}

	if (saveTeDataCnt >= 112)
	{
		saveTeDataCnt = 0;
		m_saveTeDataFile.Close();	//结束保存
	} else {
		saveTeDataCnt++;

		// 读取RAM数据
		OperDlgFlag = OPEREADSTATUS;

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x09;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x01;		//data type, date edit first byte
		TxData[4] = 0x00;		//real data
		TxData[5] = 0x00;		//预留位
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

		//Send message to main dialog
		OperCalMainMsg();		//调用主对话框串口发送消息程序
	}

}

LRESULT COperationDlg::OnOpearSaveRamDataProcess(WPARAM wParam, LPARAM lParam)
{
	CString sRamData, sSingleData;
	sRamData.Empty();
	sSingleData.Empty();

	sRamData.Format("Total valid packet number is: %0.d \r\nCurrent number is: %1.d \r\n", RxData[5], RxData[6]);

	for (int i=0; i<(RxData[3]-3); i++)
	{
		sSingleData.Format("%.2X ",RxData[i+7]);
		sRamData += sSingleData;
	}
		sRamData += "\r\n";
		
	// receive data save file
	m_saveTeDataFile.Write(sRamData,sRamData.GetLength());

	//清除数据传输buffer
	memset(RxData,0,sizeof(RxData));

	return 0;
}

extern CString sIntTime1;
extern CString sIntTime2;
extern CString sIntTime3;
extern CString sIntTime4;

void COperationDlg::OnBnClickedButtonSavjson()
{
	// TODO: Add your control notification handler code here
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };;

	GetModuleFileNameA(NULL, szFilePath, sizeof(szFilePath));

	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);

	CString sConfigFile(szDir);
	sConfigFile += CONFIGFILE;

	//construct json data
	Json::Value root;

//	int test = 23;

//	root["IntTimeCh1"] = Json::Value(test);
	//	root["IntTimeCh2"] = Json::Value(int_time2);
	//	root["GainMode"] = Json::Value(gain_mode);
	//	root["TxBin1"] = Json::Value(Txbin1);
	//	root["TxBin2"] = Json::Value(Txbin2);
	//	root["PipeMode"] = Json::Value(pipeline_mode);
	//	root["DilFactor"] = Json::Value(g_dil_factor);

	//	root["Cycle"] = Json::Value(paraConfig.Cycle);
	//	root["DenTem"] = Json::Value(paraConfig.DenTem);
	//	root["DenTim"] = Json::Value(paraConfig.DenTim);
	//	root["ExtTem"] = Json::Value(paraConfig.ExtTem);
	//	root["ExtTim"] = Json::Value(paraConfig.ExtTim);
	//	root["HoldTem"] = Json::Value(paraConfig.HoldTem);
	//	root["HoldTim"] = Json::Value(paraConfig.HoldTim);

	root["PiTemp"] = Json::Value(sPITem_write);
	root["DenTemp"] = Json::Value(sDenTem_write);
	root["DenTime"] = Json::Value(sDenTim_write);
	root["DenRamp"] = Json::Value(sDenRam_write);
	root["AnnTemp"] = Json::Value(sAnnTem_write);
	root["AnnTime"] = Json::Value(sAnnTim_write);
	root["AnnRamp"] = Json::Value(sAnnRam_write);
	root["AnnTemp2"] = Json::Value(sAnnTem2_write);
	root["AnnTime2"] = Json::Value(sAnnTim2_write);
	root["AnnRamp2"] = Json::Value(sAnnRam2_write);
	root["ExtTemp"] = Json::Value(sExtTem_write);
	root["ExtTime"] = Json::Value(sExtTim_write);
	root["ExtRamp"] = Json::Value(sExtRam_write);
	root["InitDenTemp"] = Json::Value(sInidenTem_write);
	root["InitDenTime"] = Json::Value(sInidenTim_write);
	root["HoldTemp"] = Json::Value(sHoldTem_write);
	root["HoldTime"] = Json::Value(sHoldTim_write);
	root["ANNCycle"] = Json::Value(sCycle);
	root["IntTime1"] = Json::Value(sIntTime1);
	root["IntTime2"] = Json::Value(sIntTime2);
	root["IntTime3"] = Json::Value(sIntTime3);
	root["IntTime4"] = Json::Value(sIntTime4);

	CString		otime, otemp, otime2, otemp2;
	GetDlgItemText(IDC_EDIT_OPE_OVERTIME, otime);		//overshoot set time
	GetDlgItemText(IDC_EDIT_OPE_OVERTEMP, otemp);		//overshoot set time
	GetDlgItemText(IDC_EDIT_OPE_OVERREADTIME, otime2);		//overshoot set time
	GetDlgItemText(IDC_EDIT_OPE_OVERREADTEMP, otemp2);		//overshoot set time

	root["OverTime"] = Json::Value(otime);
	root["OverTemp"] = Json::Value(otemp);
	root["OverTime2"] = Json::Value(otime2);
	root["OverTemp2"] = Json::Value(otemp2);

	root["EnAnn2"] = Json::Value(m_EnableAnnealing2);
	root["EnExt"] = Json::Value(m_EnableExtension);

	if (g_pTrimDlg) {
		root["GainMode"] = Json::Value(g_pTrimDlg->m_GainMode);
		g_pTrimDlg->TrimGetPCRMaskStatus();
		root["PCRMask"] = Json::Value(PCRMask);
	}

	root["LogThreshold"] = Json::Value(log_threshold);

	Json::FastWriter json_writer;

	std::string strJsonResult = json_writer.write(root);

	//Save Config to file
	std::ofstream outfile;

	outfile.open(sConfigFile);
	outfile << strJsonResult;
	outfile.close();
}


void COperationDlg::OnBnClickedButtonLdjson()
{
	//get config file path
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };;
	GetModuleFileNameA(NULL, szFilePath, sizeof(szFilePath));

	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
	CString sConfigFile(szDir);
	sConfigFile += CONFIGFILE;

	//wether the config file is existing or not
	DWORD dwAttrib = GetFileAttributes(sConfigFile);
	if (INVALID_FILE_ATTRIBUTES == dwAttrib || 1 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{//file not exist
		AfxMessageBox("Can't find the Json file");
		return;
	}

	//Parse the config file
	// CParaConfig paraConfig = 

	ParseJsonFromFile(sConfigFile);

	SetDlgItemTextA(IDC_EDIT_OPE_PITEM, sPITem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM, sDenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME, sDenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGRAMP, sDenRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM, sAnnTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME, sAnnTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP, sAnnRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM2, sAnnTem2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME2, sAnnTim2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP2, sAnnRam2_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM, sExtTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME, sExtTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTRAMP, sExtRam_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTEM, sInidenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTIME, sInidenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM, sHoldTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME, sHoldTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE, sCycle);

	if(g_pTrimDlg) g_pTrimDlg->SetIntTimeEdit1(sIntTime1);
	if(g_pTrimDlg) g_pTrimDlg->SetIntTimeEdit2(sIntTime2);	
	if(g_pTrimDlg) g_pTrimDlg->SetIntTimeEdit3(sIntTime3);	
	if(g_pTrimDlg) g_pTrimDlg->SetIntTimeEdit4(sIntTime4);

	if (g_pTrimDlg) g_pTrimDlg->SetPCRMask(PCRMask);

/*
	CString kstg;
	//	float kfl;

	kstg.Format("%4.3f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME, kstg);

	kstg.Format("%4.3f", int_time2);
	SetDlgItemText(IDC_EDIT_IGTIME2, kstg);

	SetIntTimeParam(1);
	SetIntTimeParam(2);

	int i;

	for (i = 0; i<LAST_PCR; i++) {
		SetGainMode(gain_mode, i + 1);
	}

	if (g_pTrimReader) {

		for (i = 0; i < LAST_PCR; i++) {
			if (!gain_mode)
				SetV20(g_pTrimReader->Node[i].auto_v20[1], i + 1);
			else
				SetV20(g_pTrimReader->Node[i].auto_v20[0], i + 1);
		}
	}

	m_ComboxTX.SetCurSel((char)Txbin1);
	m_ComboxTX2.SetCurSel((char)Txbin2);

	SelSensor(1);
	SetTXbin((char)Txbin1);

	SelSensor(2);
	SetTXbin((char)Txbin2);
	*/
}

// parse para from file
void COperationDlg::ParseJsonFromFile(CString filename)
{
	// 解析json用Json::Reader   
	Json::Reader reader;
	// Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...   
	Json::Value root;
	//	CParaConfig paraConfig;
	std::ifstream is;
	is.open(filename, std::ios::binary);

	if (reader.parse(is, root))
	{
		float	tempFloat = 1;
		int		tempInt = 0;
//		tempFloat = root.get("IntTimeCh1", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
//		float int_time = tempFloat;

		std::string tmpStr;
		BOOL tmpBool = false;

		tmpStr = root.get("PiTemp", tmpStr).asString();
		sPITem_write = tmpStr.c_str();

		tmpStr = root.get("DenTemp", tmpStr).asString();
		sDenTem_write = tmpStr.c_str();

		tmpStr = root.get("DenTime", tmpStr).asString();
		sDenTim_write = tmpStr.c_str();

		tmpStr = root.get("DenRamp", tmpStr).asString();
		sDenRam_write = tmpStr.c_str();

		tmpStr = root.get("AnnTemp", tmpStr).asString();
		sAnnTem_write = tmpStr.c_str();

		tmpStr = root.get("AnnTime", tmpStr).asString();
		sAnnTim_write = tmpStr.c_str();

		tmpStr = root.get("AnnRamp", tmpStr).asString();
		sAnnRam_write = tmpStr.c_str();

		tmpStr = root.get("AnnTemp2", tmpStr).asString();
		sAnnTem2_write = tmpStr.c_str();

		tmpStr = root.get("AnnTime2", tmpStr).asString();
		sAnnTim2_write = tmpStr.c_str();

		tmpStr = root.get("AnnRamp2", tmpStr).asString();
		sAnnRam2_write = tmpStr.c_str();

		tmpStr = root.get("ExtTemp", tmpStr).asString();
		sExtTem_write = tmpStr.c_str();

		tmpStr = root.get("ExtTime", tmpStr).asString();
		sExtTim_write = tmpStr.c_str();

		tmpStr = root.get("ExtRamp", tmpStr).asString();
		sExtRam_write = tmpStr.c_str();

		tmpStr = root.get("InitDenTemp", tmpStr).asString();
		sInidenTem_write = tmpStr.c_str();

		tmpStr = root.get("InitDenTime", tmpStr).asString();
		sInidenTim_write = tmpStr.c_str();

		tmpStr = root.get("HoldTemp", tmpStr).asString();
		sHoldTem_write = tmpStr.c_str();

		tmpStr = root.get("HoldTime", tmpStr).asString();
		sHoldTim_write = tmpStr.c_str();

		tmpStr = root.get("ANNCycle", tmpStr).asString();
		sCycle = tmpStr.c_str();

		tmpStr = root.get("IntTime1", tmpStr).asString();
		sIntTime1 = tmpStr.c_str();

		tmpStr = root.get("IntTime2", tmpStr).asString();
		sIntTime2 = tmpStr.c_str();

		tmpStr = root.get("IntTime3", tmpStr).asString();
		sIntTime3 = tmpStr.c_str();

		tmpStr = root.get("IntTime4", tmpStr).asString();
		sIntTime4 = tmpStr.c_str();

		tmpStr = root.get("OverTime", tmpStr).asString();
		((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERTIME))->SetWindowText(tmpStr.c_str());

		tmpStr = root.get("OverTemp", tmpStr).asString();
		((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERTEMP))->SetWindowText(tmpStr.c_str());

		tmpStr = root.get("OverTime2", tmpStr).asString();
		((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERREADTIME))->SetWindowText(tmpStr.c_str());

		tmpStr = root.get("OverTemp2", tmpStr).asString();
		((CEdit *)GetDlgItem(IDC_EDIT_OPE_OVERREADTEMP))->SetWindowText(tmpStr.c_str());

		tmpBool = root.get("EnAnn2", tmpBool).asBool();
		((CButton *)GetDlgItem(IDC_CHECK_ANN2))->SetCheck(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTEM2)->EnableWindow(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTIME2)->EnableWindow(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP2)->EnableWindow(tmpBool);

		tmpBool = root.get("EnExt", tmpBool).asBool();
		((CButton *)GetDlgItem(IDC_CHECK_ENEXT))->SetCheck(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTEM)->EnableWindow(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTIME)->EnableWindow(tmpBool);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTRAMP)->EnableWindow(tmpBool);

		tempFloat = root.get("LogThreshold", tempFloat).asFloat();			// Return the member named key if it exist, tempFloat otherwise.
		log_threshold = tempFloat;


/*		tempFloat = root.get("IntTimeCh2", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		int_time2 = tempFloat;

		tempFloat = 10;
		tempFloat = root.get("DilFactor", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		g_dil_factor = tempFloat;
*/

		tempInt = root.get("GainMode", tempInt).asInt();
		int m_gain_mode = tempInt;

		if (g_pTrimDlg) g_pTrimDlg->SetMGainMode(m_gain_mode);

		tempInt = root.get("PCRMask", tempInt).asInt();
		PCRMask = tempInt;

/*
		tempInt = root.get("TxBin1", tempInt).asInt();
		Txbin1 = tempInt;

		tempInt = root.get("TxBin2", tempInt).asInt();
		Txbin2 = tempInt;

		tempInt = 0;
		tempInt = root.get("PipeMode", tempInt).asInt();
		pipeline_mode = tempInt;
*/
	}

	is.close();

	//	return paraConfig;
}

void COperationDlg::OnEnKillfocusEditOpeAnntem2()
{
	// TODO: Add your control notification handler code here
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTEM2, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 120) | (fTemp < 20)) {
		MessageBox("Please set value between 20 and 120");
	}
	else {
		sAnnTem2_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM2, sAnnTem2_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnntime2()
{
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME2, sTemp);

	int fTemp;
	fTemp = (int)atoi(sTemp);

	if ((fTemp > 1200) | (fTemp < 1)) {
		MessageBox("Please set value between 1 and 1200");
	}
	else {
		sAnnTim2_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME2, sAnnTim2_write);
}


void COperationDlg::OnEnKillfocusEditOpeDeningramp()
{
	// TODO: Add your control notification handler code here
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENINGRAMP, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 5) | (fTemp < 1)) {
		MessageBox("Please set value between 1 and 5");
	}
	else {
		sDenRam_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_DENINGRAMP, sDenRam_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnnramp()
{
	// TODO: Add your control notification handler code here
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNRAMP, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 5) | (fTemp < 1)) {
		MessageBox("Please set value between 1 and 5");
	}
	else {
		sAnnRam_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP, sAnnRam_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnnramp2()
{
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNRAMP2, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 5) | (fTemp < 1)) {
		MessageBox("Please set value between 1 and 5");
	}
	else {
		sAnnRam2_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_ANNRAMP2, sAnnRam2_write);
}


void COperationDlg::OnEnKillfocusEditOpeExtramp()
{
	// TODO: Add your control notification handler code here
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_EXTRAMP, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

	if ((fTemp > 5) | (fTemp < 1)) {
		MessageBox("Please set value between 1 and 5");
	}
	else {
		sExtRam_write = sTemp;
		UpdateSimTempCurve();
	}

	SetDlgItemTextA(IDC_EDIT_OPE_EXTRAMP, sExtRam_write);
}

void COperationDlg::OnBnClickedCheckAnn2()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (m_EnableAnnealing2) {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTEM2)->EnableWindow(true);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTIME2)->EnableWindow(true);
		if(!m_bMaxRamp) (CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP2)->EnableWindow(true);
	}
	else {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTEM2)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNTIME2)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP2)->EnableWindow(false);
	}
}

void COperationDlg::OnBnClickedCheckEnext()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (m_EnableExtension) {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTEM)->EnableWindow(true);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTIME)->EnableWindow(true);
		if (!m_bMaxRamp) (CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTRAMP)->EnableWindow(true);
	}
	else {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTEM)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTTIME)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTRAMP)->EnableWindow(false);
	}

	UpdateSimTempCurve();
}


void COperationDlg::OnBnClickedCheckMaxramp()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (!m_bMaxRamp) {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_DENINGRAMP)->EnableWindow(true);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP)->EnableWindow(true);
		if(m_EnableExtension) (CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTRAMP)->EnableWindow(true);
		if(m_EnableAnnealing2) (CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP2)->EnableWindow(true);
	}
	else {
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_DENINGRAMP)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_EXTRAMP)->EnableWindow(false);
		(CEdit *)GetDlgItem(IDC_EDIT_OPE_ANNRAMP2)->EnableWindow(false);
	}

	UpdateSimTempCurve();
}

#include "TempCtrlModel.h"

void COperationDlg::OnBnClickedButton1()
{
	UpdateSimTempCurve();
}

void COperationDlg::InitSimTempChart()
{
	// TODO: Add your control notification handler code here

	pAxisBottom = NULL;
	pAxisLeft = NULL;
	pAxisBottom = m_TChart.CreateStandardAxis(CChartCtrl::BottomAxis);
	pAxisBottom->SetAutomatic(true);
	pAxisLeft = m_TChart.CreateStandardAxis(CChartCtrl::LeftAxis);
	pAxisLeft->SetAutomatic(true);
	m_TChart.GetLegend()->SetVisible(true);


	//	isInitialized = true;

	//========================
/*
	double x[] = { 1, 2, 3, 4, 5 };
	double y[] = { 2, 3, 4, 5, 3 };

	m_TChart.RemoveAllSeries();	//

	int iy = 0;

	m_pChartLines[iy] = m_TChart.CreateLineSerie();
	m_pChartLines[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
	m_pChartLines[iy]->AddPoints(&x[0], &y[0], 5);

	CString chan("Temper");
	chan.AppendFormat(" %d ", 1 + 1);
	chan.Append(": NEG");

	m_pChartLines[iy]->SetName(chan.GetBuffer());

	m_pChartLines[iy]->SetPenStyle(PS_SOLID);

	int frameindex = 0;

	m_pChartLines[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
		255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));
	m_pChartLines[iy]->SetVisible(true);

*/

	//========================

}


void COperationDlg::OnBnClickedButton3()
{
	UpdateSimTempCurve();
}

void  COperationDlg::UpdateSimTempCurve()
{
	m_TChart.RemoveAllSeries();				//

	CString stime, stempture, sramp;
	float ftempture, framp, den_temp, den_ramp;
	int itime;

	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM, stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME, stime);
	GetDlgItemText(IDC_EDIT_OPE_DENINGRAMP, sramp);

	ftempture = (float)atof(stempture);		//
	itime = atoi(stime);					//
	framp = (float)atof(sramp);

	if (m_bMaxRamp) framp = -1;

	den_temp = ftempture;
	den_ramp = framp;

	TempCtrlModel tm;
	float cyc_time;

	tm.set_init_temp(72);

	tm.sim_step(72, 3, -1, 0, 0);

	cyc_time = tm.sim_step(ftempture, (float)itime, framp, 5, 4);

	GetDlgItemText(IDC_EDIT_OPE_ANNTEM, stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME, stime);
	GetDlgItemText(IDC_EDIT_OPE_ANNRAMP, sramp);
	ftempture = (float)atof(stempture);		//
	itime = atoi(stime);					//
	framp = (float)atof(sramp);

	if (m_bMaxRamp) framp = -1;

	cyc_time += tm.sim_step(ftempture, (float)itime, framp, 1, 1);

	GetDlgItemText(IDC_EDIT_OPE_EXTTEM, stempture);		//
	GetDlgItemText(IDC_EDIT_OPE_EXTTIME, stime);
	GetDlgItemText(IDC_EDIT_OPE_EXTRAMP, sramp);

	ftempture = (float)atof(stempture);					//
	itime = atoi(stime);								//
	framp = (float)atof(sramp);

	if (m_bMaxRamp) framp = -1;

	if(m_EnableExtension) cyc_time += tm.sim_step(ftempture, (float)itime, framp, 1, 1);

	tm.sim_step(den_temp, 2, den_ramp, 5, 4);

	int iy = 0;

	m_pChartLines[iy] = m_TChart.CreateLineSerie();
	m_pChartLines[iy]->SetSeriesOrdering(poNoOrdering);								//设置为无序
	m_pChartLines[iy]->AddPoints(&tm.v_t[0], &tm.v_x[0], tm.v_t.size());

	CString chan("Temperature");
//	chan.AppendFormat(" %d ", 1 + 1);
//	chan.Append(": NEG");

	m_pChartLines[iy]->SetName(chan.GetBuffer());

	BOOL dot = false;

	m_pChartLines[iy]->SetWidth(3);
	if (dot)
		m_pChartLines[iy]->SetPenStyle(PS_DOT);
	else
		m_pChartLines[iy]->SetPenStyle(PS_SOLID);

	int frameindex = 0;

	m_pChartLines[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
		255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));
	m_pChartLines[iy]->SetVisible(true);

	// ============

	iy = 1;

	m_pChartLines[iy] = m_TChart.CreateLineSerie();
	m_pChartLines[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
	m_pChartLines[iy]->AddPoints(&tm.v_t[0], &tm.v_s[0], tm.v_t.size());

	CString chan2("Set point");
//	chan2.AppendFormat(" %d ", 2 + 1);
//	chan2.Append(chanName[iy]);
//	chan2.Append(": NEG");

	m_pChartLines[iy]->SetName(chan2.GetBuffer());

	// dot = true;
	frameindex = 3;

	m_pChartLines[iy]->SetWidth(2);

	if (dot)
		m_pChartLines[iy]->SetPenStyle(PS_DOT);
	else
		m_pChartLines[iy]->SetPenStyle(PS_SOLID);

	m_pChartLines[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
		255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));

	m_pChartLines[iy]->SetVisible(true);

	pAxisLeft->SetMinMax(55, den_temp + 6);

	m_TChart.RefreshCtrl();

	CString str;

	str.Format("%3.0f", round(cyc_time));

	SetDlgItemTextA(IDC_EDIT_EST_CYCTIME, str);
}
