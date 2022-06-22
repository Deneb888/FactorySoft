// RegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "RegDlg.h"
#include "afxdialogex.h"
#include <fstream>
using namespace std;

//*****************************************************************
//Constant definition
//*****************************************************************

#define FanOn	0x01
#define FanOff	0x00

//***************************************************************
//Global variable definition
//***************************************************************
int RegFlag;	// register dialog flag
int TimerFlag;	// timer flag
int T1Flag = 0;	// timer1 flag
int T2Flag = 0;	// timer2 flag

BYTE RegBuf [regdatanum];	// register dialog transmitted data buffer

//*****************************************************************
//External variable definition
//*****************************************************************

extern int mRegFlag;			//主对话框发送消息标志


// CRegDlg dialog

IMPLEMENT_DYNAMIC(CRegDlg, CDialogEx)

CRegDlg::CRegDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRegDlg::IDD, pParent)
{

	//  m_EditRowNum = _T("");
	m_EditKP = _T("");
	m_fanstate = _T("");
	m_EditKI = _T("");
	m_EditKL = _T("");
	m_EditInitemp = _T("");
	m_EditInitim = _T("");
	m_EditKD = _T("");
	m_EditDentemp = _T("");
	m_EditDentim = _T("");
	m_EditAnntemp = _T("");
	m_EditAnntim = _T("");
	m_EditInextentemp = _T("");
	m_EditInextentim = _T("");
	m_EditExextentemp = _T("");
	m_EditExextentim = _T("");
	m_EditCyclenum = _T("");
}

CRegDlg::~CRegDlg()
{
}

void CRegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_Text(pDX, IDC_EDIT_ROWNUM, m_EditRowNum);
	DDX_Text(pDX, IDC_EDIT_KP, m_EditKP);
	DDX_Text(pDX, IDC_EDIT_FAN_STATE, m_fanstate);
	DDX_Text(pDX, IDC_EDIT_KI, m_EditKI);
	DDX_Text(pDX, IDC_EDIT_KL, m_EditKL);
	DDX_Text(pDX, IDC_EDIT_INITEMP, m_EditInitemp);
	DDX_Text(pDX, IDC_EDIT_INITIM, m_EditInitim);
	DDX_Text(pDX, IDC_EDIT_KD, m_EditKD);
	DDX_Text(pDX, IDC_EDIT_DENTEMP, m_EditDentemp);
	DDX_Text(pDX, IDC_EDIT_DENTIM, m_EditDentim);
	DDX_Text(pDX, IDC_EDIT_ANNTEMP, m_EditAnntemp);
	//  DDX_Control(pDX, IDC_EDIT_ANNTIM, m_EditAnntim);
	DDX_Text(pDX, IDC_EDIT_ANNTIM, m_EditAnntim);
	DDX_Text(pDX, IDC_EDIT_INEXTENTEMP, m_EditInextentemp);
	DDX_Text(pDX, IDC_EDIT_INEXTENTIM, m_EditInextentim);
	DDX_Text(pDX, IDC_EDIT_EXEXTENTEMP, m_EditExextentemp);
	DDX_Text(pDX, IDC_EDIT_EXEXTENTIM, m_EditExextentim);
	DDX_Text(pDX, IDC_EDIT_CYCLENUM, m_EditCyclenum);
}


BEGIN_MESSAGE_MAP(CRegDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_KP, &CRegDlg::OnBnClickedBtnKp)
	ON_BN_CLICKED(IDC_BTN_KI, &CRegDlg::OnBnClickedBtnKi)
	ON_BN_CLICKED(IDC_BTN_KD, &CRegDlg::OnBnClickedBtnKd)
	ON_BN_CLICKED(IDC_BTN_KL, &CRegDlg::OnBnClickedBtnKl)
	ON_BN_CLICKED(IDC_BTN_P1START, &CRegDlg::OnBnClickedBtnP1start)
	ON_BN_CLICKED(IDC_BTN_P1STOP, &CRegDlg::OnBnClickedBtnP1stop)
	ON_BN_CLICKED(IDC_BTN_P2START, &CRegDlg::OnBnClickedBtnP2start)
	ON_BN_CLICKED(IDC_BTN_P2STOP, &CRegDlg::OnBnClickedBtnP2stop)
	ON_BN_CLICKED(IDC_BTN_HTSTART, &CRegDlg::OnBnClickedBtnHtstart)
	ON_BN_CLICKED(IDC_BTN_HTSTOP, &CRegDlg::OnBnClickedBtnHtstop)
	ON_BN_CLICKED(IDC_BTN_CHIP, &CRegDlg::OnBnClickedBtnChip)
	ON_BN_CLICKED(IDC_BTN_PELTIER1, &CRegDlg::OnBnClickedBtnPeltier1)
	ON_BN_CLICKED(IDC_BTN_PELTIER2, &CRegDlg::OnBnClickedBtnPeltier2)
	ON_MESSAGE(UM_REGPROCESS,OnRegProcess)
	ON_BN_CLICKED(IDC_BTN_TEMPSET, &CRegDlg::OnBnClickedBtnTempset)
	ON_BN_CLICKED(IDC_BTN_TEMPSTAR, &CRegDlg::OnBnClickedBtnTempstar)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_P1CLEAR, &CRegDlg::OnBnClickedBtnP1clear)
	ON_BN_CLICKED(IDC_BTN_P2CLEAR, &CRegDlg::OnBnClickedBtnP2clear)
	ON_BN_CLICKED(IDC_BTN_P1SAVE, &CRegDlg::OnBnClickedBtnP1save)
	ON_BN_CLICKED(IDC_BTN_P2SAVE, &CRegDlg::OnBnClickedBtnP2save)
	ON_BN_CLICKED(IDC_BTN_FANCON_AUTO, &CRegDlg::OnBnClickedBtnFanconAuto)
	ON_BN_CLICKED(IDC_BTN_FANCON_ON, &CRegDlg::OnBnClickedBtnFanconOn)
	ON_BN_CLICKED(IDC_BTN_FANCON_OFF, &CRegDlg::OnBnClickedBtnFanconOff)
	ON_EN_CHANGE(IDC_EDIT_PELTIER2, &CRegDlg::OnEnChangeEditPeltier2)
	ON_BN_CLICKED(IDC_BTN_FANCON_STATE, &CRegDlg::OnBnClickedBtnFanconState)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_EDIT_FAN_STATE, &CRegDlg::OnEnChangeEditFanState)
	ON_BN_CLICKED(IDC_BTN_FANTEMP, &CRegDlg::OnBnClickedBtnFantemp)
	ON_BN_CLICKED(IDC_BTN_LEDLIGHT, &CRegDlg::OnBnClickedBtnLedlight)
//	ON_BN_CLICKED(IDC_BTN_PID_READ, &CRegDlg::OnBnClickedBtnPidRead)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CRegDlg::OnBnClickedBtnSave)
	ON_BN_CLICKED(IDC_BTN_LOAD, &CRegDlg::OnBnClickedBtnLoad)
//	ON_BN_CLICKED(IDC_BTN_PID_READ, &CRegDlg::OnBnClickedBtnPidRead)
ON_BN_CLICKED(IDC_BTN_READ_PID1, &CRegDlg::OnBnClickedBtnReadPid1)
END_MESSAGE_MAP()

//************************************************************
//Own Function
//************************************************************

//调用主对话框对应的处理消息
void CRegDlg::RegCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
//	::SendMessage(hwnd,WM_RegDlg_event,a,b);
}

//自添加消息处理函数
LRESULT CRegDlg::OnRegProcess(WPARAM wParam,LPARAM lParam)
{
	BYTE rCmd,rType;
	rCmd = RxData[2];	//RxData[2]为返回命令
	rType = RxData[4];	//RxData[4]为各分支区别位
	BYTE rFan;
	rFan = RxData[5];

	if (rCmd == 0x10)
	{
		//将接收的4byte格式浮点数据转为浮点字符串
		CString sTem;
		unsigned char cTem[] = {RxData[5],RxData[6],RxData[7],RxData[8]};
		float * fTem = (float *)cTem;
		sTem.Format("%g",*fTem);	

		switch(rType)
		{
		case 0x01:	//接收到Peltier1返回数据
			{
				sPeltier1 += (sTem + "\r\n");
				SetDlgItemText(IDC_EDIT_PELTIER1,sPeltier1);

				//编辑框垂直滚动到底端
				POINT pt;
				GetDlgItem(IDC_EDIT_PELTIER1)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
				pt.x=0;
				GetDlgItem(IDC_EDIT_PELTIER1)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				break;
			}
		case 0x02:	//接收到Peltier2返回数据
			{
				sPeltier2 += (sTem + "\r\n");
				SetDlgItemText(IDC_EDIT_PELTIER2,sPeltier2);

				//编辑框垂直滚动到底端
				POINT pt;
				GetDlgItem(IDC_EDIT_PELTIER2)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
				pt.x=0;
				GetDlgItem(IDC_EDIT_PELTIER2)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				break;
			}

		default:
			break;
		}

		switch(rFan)
		{
		case 0x01:
		case 0x03:
			m_fanstate = "ON";
			UpdateData(FALSE);
			break;
		case 0x02:
			m_fanstate = "OFF";
			UpdateData(FALSE);
			break;
		default:
			break;
		}
	}

	// Read PID 返回处理
	if (rCmd == 0x12)
	{
		//将接收的4byte格式浮点数据转为浮点字符串
		CString sTemKP;
		unsigned char cTemKP[] = {RxData[4],RxData[5],RxData[6],RxData[7]};
		float * fTemKP = (float *)cTemKP;
		sTemKP.Format("%g",*fTemKP);

		m_EditKP = sTemKP;

		CString sTemKI;
		unsigned char cTemKI[] = {RxData[8],RxData[9],RxData[10],RxData[11]};
		float * fTemKI = (float *)cTemKI;
		sTemKI.Format("%g",*fTemKI);

		m_EditKI = sTemKI;
		
		CString sTemKL;
		unsigned char cTemKL[] = {RxData[12],RxData[13],RxData[14],RxData[15]};
		float * fTemKL = (float *)cTemKL;
		sTemKL.Format("%g",*fTemKL);

		m_EditKL = sTemKL;

		SetDlgItemText(IDC_EDIT_KP,m_EditKP);
		SetDlgItemText(IDC_EDIT_KI,m_EditKI);
		SetDlgItemText(IDC_EDIT_KL,m_EditKL);

		//清除数据传输buffer
		memset(RxData,0,sizeof(RxData));
	}

	return 0;
}



// CRegDlg message handlers


void CRegDlg::OnBnClickedBtnKp()
{
	// TODO: Add your control notification handler code here
	
	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_KP,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnKi()
{
	// TODO: Add your control notification handler code here

	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_KI,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnKd()
{
	// TODO: Add your control notification handler code here

	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_KD,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnKl()
{
	// TODO: Add your control notification handler code here

	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_KL,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnP1start()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	CString stime, stempture;
	float ftempture;
	int itime;


	GetDlgItemText(IDC_EDIT_P1TIM,stime);
	GetDlgItemText(IDC_EDIT_P1TEM,stempture);

	itime = atoi(stime);	//将tim编辑框的字符串转成整型变量
	RegBuf[4] = itime>>8;		//转换后高byte赋值给timbuf[0]
	RegBuf[5] = itime>>0;	//转换后低byte赋值给timbuf[1]

	ftempture = (float)atof(stempture);	//将tempture编辑框的字符串转成浮点型变量
	if ((ftempture > 100) | (ftempture < 10))
		MessageBox("please fill the number between 40 and 100");
	else
	{
		unsigned char * hData = (unsigned char *)&ftempture;	//将浮点数据转化为十六进制数据

		RegBuf[0] = hData[0];
		RegBuf[1] = hData[1];
		RegBuf[2] = hData[2];
		RegBuf[3] = hData[3];

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x10;		//command
		TxData[2] = 0x0C;		//data length
		TxData[3] = 0x01;		//data type, date edit first byte
		TxData[4] = 0x01;		//real data
		TxData[5] = RegBuf[0];	//tp第一字节				
		TxData[6] = RegBuf[1];	
		TxData[7] = RegBuf[2];
		TxData[8] = RegBuf[3];	//tp最后一字节
		TxData[9] = RegBuf[4];	//time低字节
		TxData[10] = RegBuf[5];	//time高字节
		TxData[11] = 0x00;		//预留位
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
		RegCalMainMsg();	//调用主对话框处理消息程序
	}
}


void CRegDlg::OnBnClickedBtnP1stop()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x00;		//data type, date edit first byte
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
	for (int i=1; i<15; i++)
		TxData[15]+=TxData[i];			//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnP2start()
{
	// TODO: Add your control notification handler code here
	RegFlag = sendregmsg;

	CString stime, stempture;
	float ftempture;
	int itime;


	GetDlgItemText(IDC_EDIT_P2TIM,stime);
	GetDlgItemText(IDC_EDIT_P2TEM,stempture);

	itime = atoi(stime);	//将tim编辑框的字符串转成整型变量
	RegBuf[4] = itime>>8;		//转换后高byte赋值给timbuf[0]
	RegBuf[5] = itime>>0;	//转换后低byte赋值给timbuf[1]

	ftempture = (float)atof(stempture);	//将tempture编辑框的字符串转成浮点型变量
	if ((ftempture > 100) | (ftempture < 10))
		MessageBox("please fill the number between 40 and 100");
	else
	{
		unsigned char * hData = (unsigned char *)&ftempture;	//将浮点数据转化为十六进制数据

		RegBuf[0] = hData[0];
		RegBuf[1] = hData[1];
		RegBuf[2] = hData[2];
		RegBuf[3] = hData[3];

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x10;		//command
		TxData[2] = 0x0C;		//data length
		TxData[3] = 0x01;		//data type, date edit first byte
		TxData[4] = 0x02;		//real data
		TxData[5] = RegBuf[0];	//tp第一字节				
		TxData[6] = RegBuf[1];	
		TxData[7] = RegBuf[2];
		TxData[8] = RegBuf[3];	//tp最后一字节
		TxData[9] = RegBuf[4];	//time低字节
		TxData[10] = RegBuf[5];	//time高字节
		TxData[11] = 0x00;		//预留位
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
		RegCalMainMsg();	//调用主对话框处理消息程序
	}
}


void CRegDlg::OnBnClickedBtnP2stop()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x00;		//data type, date edit first byte
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnHtstart()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = 0x03;		//real data
	TxData[5] = 0x00;	//tp第一字节				
	TxData[6] = 0x00;	
	TxData[7] = 0x00;
	TxData[8] = 0x00;	//tp最后一字节
	TxData[9] = 0x00;	//time低字节
	TxData[10] = 0x00;	//time高字节
	TxData[11] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnHtstop()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x00;		//data type, date edit first byte
	TxData[4] = 0x03;		//real data
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}




void CRegDlg::OnBnClickedBtnChip()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x11;		//data type, date edit first byte
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnPeltier1()
{
	// TODO: Add your control notification handler code here

	if ((TimerFlag == 0 )|(T2Flag == 1))		//TimerFlag:表示Timer是否已经打开，0为没打开，1为打开
											//T2Flag表示Timer2是否打开，0为没打开，1为打开
	{
		T1Flag = 1;
		SetTimer(1,1000,NULL);	//设置连续发送的timer1
	}		
	else
	{
		T1Flag = 0;
		KillTimer(1);
	}			
	if (TimerFlag==1)
		TimerFlag = 0;	

}


void CRegDlg::OnBnClickedBtnPeltier2()
{
	// TODO: Add your control notification handler code here

	if ((TimerFlag == 0 )|(T1Flag == 1))
	{
		T2Flag = 1;
		SetTimer(2,1000,NULL);
	}		
	else
	{
		T2Flag =0;
		KillTimer(2);
	}
	if (TimerFlag==1)
		TimerFlag = 0;	

}


void CRegDlg::OnBnClickedBtnTempset()
{
	// TODO: Add your control notification handler code here
	
	RegFlag = sendregmsg;

	CString stime, stempture;
	float ftempture;
	int itime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_DENTEMP,stempture);		//取Dennature编辑框中的数据
	GetDlgItemText(IDC_EDIT_DENTIM,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	RegBuf[0] = hData[0];
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	RegBuf[4] = itime>>8;
	RegBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_ANNTEMP,stempture);		//取Anneal编辑框中的数据
	GetDlgItemText(IDC_EDIT_ANNTIM,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	RegBuf[6] = hData[0];
	RegBuf[7] = hData[1];
	RegBuf[8] = hData[2];
	RegBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	RegBuf[10] = itime>>8;
	RegBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_INEXTENTEMP,stempture);		//取Inter Extension编辑框中的数据
	GetDlgItemText(IDC_EDIT_INEXTENTIM,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	RegBuf[12] = hData[0];
	RegBuf[13] = hData[1];
	RegBuf[14] = hData[2];
	RegBuf[15] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	RegBuf[16] = itime>>8;
	RegBuf[17] = itime;

	GetDlgItemText(IDC_EDIT_STAGECNT,stime);
	itime = atoi(stime);
	RegBuf[18] = itime;		//stage count编辑框值

	GetDlgItemText(IDC_EDIT_CYCLENUM,stime);
	itime = atoi(stime);
	RegBuf[19] = itime;		//cycle编辑框值

	GetDlgItemText(IDC_EDIT_STAGENUM,stime);
	itime = atoi(stime);
	RegBuf[20] = itime;		//stage no.编辑框值

	GetDlgItemText(IDC_EDIT_STEPNUM,stime);
	itime = atoi(stime);
	RegBuf[21] = itime;		//step no.编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x17;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte TXC
	//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;		//RegBuf[18];						
	TxData[5] = 0x1;        //RegBuf[20];	
	TxData[6] = 0x03;       // RegBuf[21];
	TxData[7] = RegBuf[0];		//dennature数据
	TxData[8] = RegBuf[1];	
	TxData[9] = RegBuf[2];	
	TxData[10] = RegBuf[3];		
	TxData[11] = RegBuf[4];
	TxData[12] = RegBuf[5];
	TxData[13] = RegBuf[6];		//Anneal数据
	TxData[14] = RegBuf[7];
	TxData[15] = RegBuf[8];
	TxData[16] = RegBuf[9];
	TxData[17] = RegBuf[10];
	TxData[18] = RegBuf[11];
	TxData[19] = RegBuf[12];	//Inter extension数据
	TxData[20] = RegBuf[13];	
	TxData[21] = RegBuf[14];	
	TxData[22] = RegBuf[15];	
	TxData[23] = RegBuf[16];	
	TxData[24] = RegBuf[17];
	for (int i=1; i<25; i++)
		TxData[25] += TxData[i];
	if (TxData[25]==0x17)
		TxData[25]=0x18;
	else
		TxData[25]=TxData[25];
	TxData[26]=0x17;
	TxData[27]=0x17;
	
	//Send message to main dialog
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnTempstar()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	CString stime, stempture;
	float ftempture;
	int itime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_INITEMP,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_INITIM,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	RegBuf[0] = hData[0];
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	RegBuf[4] = itime>>8;
	RegBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_EXEXTENTEMP,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_EXEXTENTIM,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	RegBuf[6] = hData[0];
	RegBuf[7] = hData[1];
	RegBuf[8] = hData[2];
	RegBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	RegBuf[10] = itime>>8;
	RegBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_CYCLENUM,stime);
	itime = atoi(stime);
	RegBuf[12] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x10;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte TXC
	TxData[4] = 0x01;		//real data
	TxData[5] = RegBuf[12];	
	TxData[6] = 0x00;       //启动
	TxData[7] = RegBuf[0];	//inital dennature数据	
	TxData[8] = RegBuf[1];  //
	TxData[9] =	RegBuf[2];		
	TxData[10] = RegBuf[3];	
	TxData[11] = RegBuf[4];	
	TxData[12] = RegBuf[5];		
	TxData[13] = RegBuf[6];	//extern extension数据
	TxData[14] = RegBuf[7];
	TxData[15] = RegBuf[8];
	TxData[16] = RegBuf[9];
	TxData[17] = RegBuf[10];
	TxData[18] = RegBuf[11];
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
	RegCalMainMsg();	//调用主对话框处理消息程序

}


void CRegDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	TimerFlag=1;

	if (T1Flag&&T2Flag)		// timer1和timer2已同时打开
	{
		T1Flag = 0;			// timer1/2 flag 清零
		T2Flag = 0;
	}

	switch (nIDEvent)
	{

		case 1 :		//peltier1 send command
			{
				RegFlag = sendregmsg;

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
				RegCalMainMsg();	//调用主对话框处理消息程序

				break;
			}
		case 2:		//peltier2 send command
			{
				RegFlag = sendregmsg;

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
				RegCalMainMsg();	//调用主对话框处理消息程序

				break;
			}

		default:break;
	}
	
//	CDialogEx::OnTimer(nIDEvent);
}


void CRegDlg::OnBnClickedBtnP1clear()
{
	// TODO: Add your control notification handler code here
	
	sPeltier1 = "";
	SetDlgItemText(IDC_EDIT_PELTIER1,sPeltier1);
}


void CRegDlg::OnBnClickedBtnP2clear()
{
	// TODO: Add your control notification handler code here
	
	sPeltier2 = "";
	SetDlgItemText(IDC_EDIT_PELTIER2,sPeltier2);
}


void CRegDlg::OnBnClickedBtnP1save()
{
	// TODO: Add your control notification handler code here

	CFileDialog tDlg(false,"txt",NULL,0,"Excel文件(*.txt)|*.txt||");

	if (tDlg.DoModal()==IDOK)
	{
		CString nFilename;
		nFilename=tDlg.GetPathName();
		CFile nFile(nFilename, CFile::modeCreate|CFile::modeReadWrite);
		CStatic*pst=(CStatic*)GetDlgItem(IDC_EDIT_PELTIER1);

		CString str;
		pst->GetWindowText(str);

		CArchive save(&nFile, CArchive::store|CArchive::bNoFlushOnDelete);
		save<<str;

		save.Close();
	}

}


void CRegDlg::OnBnClickedBtnP2save()
{
	// TODO: Add your control notification handler code here

	CFileDialog tDlg(false,"txt",NULL,0,"Excel文件(*.txt)|*.txt||");

	if (tDlg.DoModal()==IDOK)
	{
		CString nFilename;
		nFilename=tDlg.GetPathName();
		CFile nFile(nFilename, CFile::modeCreate|CFile::modeReadWrite);
		CStatic*pst=(CStatic*)GetDlgItem(IDC_EDIT_PELTIER2);

		CString str;
		pst->GetWindowText(str);

		CArchive save(&nFile, CArchive::store|CArchive::bNoFlushOnDelete);
		save<<str;

		save.Close();
	}
}


void CRegDlg::OnBnClickedBtnFanconAuto()
{
	// TODO: 在此添加控件通知处理程序代码
	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = 0x02;		//real data

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();
}


void CRegDlg::OnBnClickedBtnFanconOn()
{
	// TODO: 在此添加控件通知处理程序代码
	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = 0x01;		//real data

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();
}


void CRegDlg::OnBnClickedBtnFanconOff()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = 0x00;		//real data

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();
}


void CRegDlg::OnEnChangeEditPeltier2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CRegDlg::OnBnClickedBtnFanconState()
{
	// TODO: 在此添加控件通知处理程序代码
	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x0a;		//data type, date edit first byte
	TxData[4] = 0x0F;		//real data

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();
}


BOOL CRegDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();  

	// TODO:  Add extra initialization here

	//初始化风扇编辑框显示
	m_fanstate = "OFF";
	UpdateData(FALSE);

	//初始化P1、P2编辑框画刷
	m_editbrush.CreateSolidBrush(RGB(255,160,0));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


HBRUSH CRegDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	if (pWnd->GetDlgCtrlID()==IDC_EDIT_PELTIER1)
	{
		pDC->SetBkColor(RGB(255,160,0));
		hbr=(HBRUSH)m_editbrush;
	}
	if (pWnd->GetDlgCtrlID()==IDC_EDIT_PELTIER2)
	{
		pDC->SetBkColor(RGB(255,160,0));
		hbr=m_editbrush;
	}

	return hbr;
}


void CRegDlg::OnEnChangeEditFanState()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CRegDlg::OnBnClickedBtnFantemp()
{
	// TODO: Add your control notification handler code here
	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_FANTEMP,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnLedlight()
{
	// TODO: Add your control notification handler code here
	RegFlag= sendregmsg;

	//取KP编辑框的值传给主对话框
	CString kstg;
	float kfl;

	GetDlgItemText(IDC_EDIT_LEDLIGHT,kstg);		//从编辑框获取数值字符串

	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据

	RegBuf[0] = hData[0];	//存到窗口传递buffer
	RegBuf[1] = hData[1];
	RegBuf[2] = hData[2];
	RegBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = RegBuf[0];		//real data
	TxData[5] = RegBuf[1];		
	TxData[6] = RegBuf[2];
	TxData[7] = RegBuf[3];
	TxData[8] = 0x00;		//预留位
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
	RegCalMainMsg();	//调用主对话框处理消息程序
}


void CRegDlg::OnBnClickedBtnSave()
{
	// TODO: Add your control notification handler code here

	CString sSave;

	UpdateData(TRUE);

	sSave = m_EditInitemp + "\r\n"+ m_EditInitim + "\r\n"+ m_EditDentemp + "\r\n"+ m_EditDentim
			+ "\r\n"+ m_EditAnntemp + "\r\n"+ m_EditAnntim + "\r\n"+ m_EditInextentemp + "\r\n"+ m_EditInextentim
			+ "\r\n"+ m_EditExextentemp + "\r\n"+ m_EditExextentim + "\r\n"+ m_EditCyclenum
			+ "\r\n"+ m_EditKP + "\r\n"+ m_EditKI + "\r\n"+ m_EditKD + "\r\n" + m_EditKL + "\r\n";

	CFileDialog saveDlg(FALSE,".txt",NULL,NULL,"(*.txt)|*.txt|");

	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		ofs << sSave;
	}

}


void CRegDlg::OnBnClickedBtnLoad()
{
	// TODO: Add your control notification handler code here

	UpdateData(FALSE);
}



void CRegDlg::OnBnClickedBtnReadPid1()
{
	// TODO: Add your control notification handler code here

	RegFlag = sendregmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x12;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x00;		//data type, date edit first byte
	TxData[4] = 0x00;		//real data

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Send message to main dialog
	RegCalMainMsg();
}
