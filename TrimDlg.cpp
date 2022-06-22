// TrimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "TrimDlg.h"
#include "IntTimeDlg.h"
#include "SerialNumDlg.h"
#include "afxdialogex.h"

#include <fstream>
//#include <afx.h>
//#include <afxres.h>

//added by lixin 20190327
#include "MathRound.cpp"

using namespace std;

//*****************************************************************
//Constant definition
//*****************************************************************


//***************************************************************
//Global variable definition
//***************************************************************
int TrimFlag;	// register dialog flag

BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

BYTE TimCom;		//commandÏÂÀ­¿òµÄÈ¡Öµbuffer
BYTE TimType;		//typeÏÂÀ­¿òµÄÈ¡Öµbuffer

BYTE SensorRadioValue;		// sensor radio select value

//*****************************************************************
//TrimDlg global function definition
//*****************************************************************


//*****************************************************************
//External variable definition
//*****************************************************************
extern CTrimReader *g_pTrimReader;
extern DPReader *g_pDPReader;

//****************************************************
//External function 
//****************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
extern int ChangeNum (CString str, int length);									//Ê®Áù½øÖÆ×Ö·û´®×ªÊ®½øÖÆÕûÐÍ
extern  char* EditDataCvtChar (CString strCnv,  char * charRec);	//±à¼­¿òÈ¡Öµ×ª×Ö·û±äÁ¿

int gain_mode = 0;			// 0 high gain, 1 low gain. HW default is high gain
// float int_time = 1.0;	// zd add, in ms
int frame_size = 0;			//12x12 default

extern BOOL g_DeviceDetected;

int PCRMask = 0x1;					// chan mask

CString sIntTime1 = "1";
CString sIntTime2 = "1";
CString sIntTime3 = "1";
CString sIntTime4 = "1";

float int_time1 = 1;
float int_time2 = 1;
float int_time3 = 1;
float int_time4 = 1;

float cur_factor1 = 1;
float cur_factor2 = 1;
float cur_factor3 = 1;
float cur_factor4 = 1;

BYTE EepromBuff[16 + 4 * NUM_EPKT][EPKT_SZ + 1];		// 16 pages maximum - enough to support 16 well 4 channel.

extern CGraDlg *g_pGraDlg;

// CTrimDlg dialog

IMPLEMENT_DYNAMIC(CTrimDlg, CDialog)

CTrimDlg::CTrimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrimDlg::IDD, pParent)
	, m_GainMode(0)						// GUI default low gain
	, m_FrameSize(FALSE)				// GUI default 12X12
{
	m_radio_trimdlg_isensor = 0;

	m_tc95 = 0;
	m_tc55 = 0;
}

CTrimDlg::~CTrimDlg()
{
}

void CTrimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Rampgen, m_ComboxRampgen);
	//	DDX_Control(pDX, IDC_COMBO_Rampgen2, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_RANGE, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_AmuxSel, m_ComboxAmux);
	//  DDX_Control(pDX, IDC_COMBO_CMD, m_ComboxcOM);
	DDX_Control(pDX, IDC_COMBO_IPIX, m_ComboxIpix);
	DDX_Control(pDX, IDC_COMBO_Switch, m_ComboxSwitch);
	DDX_Control(pDX, IDC_COMBO_TestADC, m_ComboxTestADC);
	DDX_Control(pDX, IDC_COMBO_TxBin, m_ComboxTX);
	//  DDX_Control(pDX, IDC_COMBO_Type, m_ComboxType);
	DDX_Control(pDX, IDC_COMBO_V15, m_ComboxV15);
	DDX_Control(pDX, IDC_COMBO_V20, m_ComboxV20);
	DDX_Control(pDX, IDC_COMBO_V24, m_ComboxV24);
	DDX_Control(pDX, IDC_EDIT_REDATA, m_EditRecData);
	DDX_Control(pDX, IDC_EDIT_SENDDATA, m_EditData);
	DDX_Radio(pDX, IDC_RADIO_TRIM_SENSOR1, m_radio_trimdlg_isensor);
	DDX_Control(pDX, IDC_COMBO_TxBin2, m_ComboxTX2);
	DDX_Control(pDX, IDC_COMBO_TxBin3, m_ComboxTX3);
	DDX_Control(pDX, IDC_COMBO_TxBin4, m_ComboxTX4);

	DDX_Radio(pDX, IDC_RADIO_LOWGAIN, m_GainMode);
	DDX_Radio(pDX, IDC_RADIO_FRAME12, m_FrameSize);
}


BEGIN_MESSAGE_MAP(CTrimDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_REREAD, &CTrimDlg::OnBnClickedBtnReread)
	ON_CBN_SELCHANGE(IDC_COMBO_Rampgen, &CTrimDlg::OnCbnSelchangeComboRampgen)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGE, &CTrimDlg::OnCbnSelchangeComboRange)
	ON_CBN_SELCHANGE(IDC_COMBO_V24, &CTrimDlg::OnCbnSelchangeComboV24)
	ON_CBN_SELCHANGE(IDC_COMBO_V20, &CTrimDlg::OnCbnSelchangeComboV20)
	ON_CBN_SELCHANGE(IDC_COMBO_V15, &CTrimDlg::OnCbnSelchangeComboV15)
	ON_CBN_SELCHANGE(IDC_COMBO_IPIX, &CTrimDlg::OnCbnSelchangeComboIpix)
	ON_CBN_SELCHANGE(IDC_COMBO_Switch, &CTrimDlg::OnCbnSelchangeComboSwitch)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin, &CTrimDlg::OnCbnSelchangeComboTxbin)
	ON_CBN_SELCHANGE(IDC_COMBO_AmuxSel, &CTrimDlg::OnCbnSelchangeComboAmuxsel)
	ON_CBN_SELCHANGE(IDC_COMBO_TestADC, &CTrimDlg::OnCbnSelchangeComboTestadc)
	ON_BN_CLICKED(IDC_BTN_SENDCMD, &CTrimDlg::OnBnClickedBtnSendcmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_CMD, &CTrimDlg::OnKillfocusComboCmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_Type, &CTrimDlg::OnKillfocusComboType)
ON_BN_CLICKED(IDC_BTN_ITGTIM, &CTrimDlg::OnBnClickedBtnItgtim)
ON_BN_CLICKED(IDC_TRIM_BTN_PCRTEMP1, &CTrimDlg::OnBnClickedTrimBtnPcrtemp1)
ON_BN_CLICKED(IDC_TRIM_BTN_PCRTEMP2, &CTrimDlg::OnBnClickedTrimBtnPcrtemp2)
	ON_MESSAGE(UM_TRIMPROCESS,OnTrimProcess)
	ON_BN_CLICKED(IDC_BTN_SHOWDATA, &CTrimDlg::OnBnClickedBtnShowdata)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR1, &CTrimDlg::OnClickedRadioTrimSensor1)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR2, &CTrimDlg::OnBnClickedRadioTrimSensor2)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR3, &CTrimDlg::OnBnClickedRadioTrimSensor3)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR4, &CTrimDlg::OnBnClickedRadioTrimSensor4)
	ON_BN_CLICKED(IDC_TRIM_BTN_LOAD, &CTrimDlg::OnBnClickedTrimBtnLoad)
	ON_BN_CLICKED(IDC_TRIM_BTN_TEST, &CTrimDlg::OnClickedTrimBtnTest)
	ON_BN_CLICKED(IDC_BTN_ITGTIM2, &CTrimDlg::OnBnClickedBtnItgtim2)
	ON_BN_CLICKED(IDC_BTN_ITGTIM3, &CTrimDlg::OnBnClickedBtnItgtim3)
	ON_BN_CLICKED(IDC_BTN_ITGTIM4, &CTrimDlg::OnBnClickedBtnItgtim4)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin2, &CTrimDlg::OnCbnSelchangeComboTxbin2)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin3, &CTrimDlg::OnCbnSelchangeComboTxbin3)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin4, &CTrimDlg::OnCbnSelchangeComboTxbin4)
	ON_BN_CLICKED(IDC_RADIO_LOWGAIN, &CTrimDlg::OnBnClickedRadioLowgain)
	ON_BN_CLICKED(IDC_RADIO_HIGHGAIN, &CTrimDlg::OnBnClickedRadioHighGain)
	ON_BN_CLICKED(IDC_RADIO_LOWGAIN, &CTrimDlg::OnBnClickedRadioLowgain)
	ON_BN_CLICKED(IDC_RADIO_HIGHGAIN, &CTrimDlg::OnBnClickedRadioHighGain)
	ON_BN_CLICKED(IDC_RADIO_FRAME12, &CTrimDlg::OnBnClickedRadioFrame12)
	ON_BN_CLICKED(IDC_RADIO_FRAME24, &CTrimDlg::OnBnClickedRadioFrame24)
	ON_EN_KILLFOCUS(IDC_TRIM_EDIT_IntergrateTime, &CTrimDlg::OnKillfocusTrimEditIntergratetime)
	ON_EN_KILLFOCUS(IDC_TRIM_EDIT_IntergrateTime2, &CTrimDlg::OnKillfocusTrimEditIntergratetime2)
	ON_EN_KILLFOCUS(IDC_TRIM_EDIT_IntergrateTime3, &CTrimDlg::OnKillfocusTrimEditIntergratetime3)
	ON_EN_KILLFOCUS(IDC_TRIM_EDIT_IntergrateTime4, &CTrimDlg::OnKillfocusTrimEditIntergratetime4)
	ON_BN_CLICKED(IDC_BUTTON_WREEP, &CTrimDlg::OnBnClickedButtonWreep)
	ON_BN_CLICKED(IDC_BUTTON_RDEEP, &CTrimDlg::OnBnClickedButtonRdeep)
	ON_BN_CLICKED(IDC_BUTTON_VERIFY, &CTrimDlg::OnBnClickedButtonVerify)
//	ON_EN_KILLFOCUS(IDC_TRIM_EDIT_PCRTEMP1, &CTrimDlg::OnKillfocusTrimEditPcrtemp1)
//	ON_COMMAND(IDC_TRIM_EDIT_TempCompen55, &CTrimDlg::OnTrimEditTempcompen55)
ON_EN_KILLFOCUS(IDC_TRIM_EDIT_TempCompen95, &CTrimDlg::OnEnKillfocusTrimEditTempcompen95)
ON_EN_KILLFOCUS(IDC_TRIM_EDIT_TempCompen55, &CTrimDlg::OnEnKillfocusTrimEditTempcompen55)
ON_BN_CLICKED(IDC_TRIM_BTN_SN, &CTrimDlg::OnBnClickedTrimBtnSn)
ON_BN_CLICKED(IDC_BUTTON_SAVEF, &CTrimDlg::OnBnClickedButtonSavef)
ON_BN_CLICKED(IDC_BUTTON_READF, &CTrimDlg::OnBnClickedButtonReadf)
END_MESSAGE_MAP()


void CTrimDlg::OnOK()
{

}


// CTrimDlg message handlers

BYTE CTrimDlg::SensorRadioSel(BYTE selBuf)
{
	UpdateData(TRUE);

	switch(m_radio_trimdlg_isensor)
	{
	case 0:
		selBuf = 0;
		break;
	case 1:
		selBuf = 1;
		break;
	case 2:
		selBuf = 2;
		break;
	case 3:
		selBuf = 3;
		break;
	default:
		break;
	}

	return selBuf;			// selBuf not used.
}

// Re-purposed to reset trim

void CTrimDlg::OnBnClickedBtnReread()
{
	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	// TODO: Add your control notification handler code here

/*	TrimFlag = SENDTRIMMSG; 
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x05;		//data type, dat edit first byte
	TxData[4] = 0x00;		//real data, data edit second byte
	TxData[5] = 0x08;		//real data, data edit third byte
	TxData[6] = 0x17;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
*/
	// TODO: Add your control notification handler code here
	ResetTrim();
}

void CTrimDlg::TrimCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_TrimDlg_event,a,b);
}

extern int numChannels;

BOOL CTrimDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	
	//Rampgen add string
	CString sRamp;

	for (unsigned int i=0; i<256; i++) // this is strange
	{
		sRamp.Format("%.2X",i);
		m_ComboxRampgen.AddString(sRamp);
	}

	//Range add string
	m_ComboxRange.AddString("0x00");
	m_ComboxRange.AddString("0x01");
	m_ComboxRange.AddString("0x02");
	m_ComboxRange.AddString("0x03");
	m_ComboxRange.AddString("0x04");
	m_ComboxRange.AddString("0x05");
	m_ComboxRange.AddString("0x06");
	m_ComboxRange.AddString("0x07");
	m_ComboxRange.AddString("0x08");
	m_ComboxRange.AddString("0x09");
	m_ComboxRange.AddString("0x0a");
	m_ComboxRange.AddString("0x0b");
	m_ComboxRange.AddString("0x0c");
	m_ComboxRange.AddString("0x0d");
	m_ComboxRange.AddString("0x0e");
	m_ComboxRange.AddString("0x0f");
	m_ComboxRange.SetCurSel(15);

	//V24 add string
	m_ComboxV24.AddString("0x00");
	m_ComboxV24.AddString("0x01");
	m_ComboxV24.AddString("0x02");
	m_ComboxV24.AddString("0x03");
	m_ComboxV24.AddString("0x04");
	m_ComboxV24.AddString("0x05");
	m_ComboxV24.AddString("0x06");
	m_ComboxV24.AddString("0x07");
	m_ComboxV24.AddString("0x08");
	m_ComboxV24.AddString("0x09");
	m_ComboxV24.AddString("0x0a");
	m_ComboxV24.AddString("0x0b");
	m_ComboxV24.AddString("0x0c");
	m_ComboxV24.AddString("0x0d");
	m_ComboxV24.AddString("0x0e");
	m_ComboxV24.AddString("0x0f");
	m_ComboxV24.SetCurSel(8);

	//V20 add string
	m_ComboxV20.AddString("0x00");
	m_ComboxV20.AddString("0x01");
	m_ComboxV20.AddString("0x02");
	m_ComboxV20.AddString("0x03");
	m_ComboxV20.AddString("0x04");
	m_ComboxV20.AddString("0x05");
	m_ComboxV20.AddString("0x06");
	m_ComboxV20.AddString("0x07");
	m_ComboxV20.AddString("0x08");
	m_ComboxV20.AddString("0x09");
	m_ComboxV20.AddString("0x0a");
	m_ComboxV20.AddString("0x0b");
	m_ComboxV20.AddString("0x0c");
	m_ComboxV20.AddString("0x0d");
	m_ComboxV20.AddString("0x0e");
	m_ComboxV20.AddString("0x0f");
	m_ComboxV20.SetCurSel(8);

	//V15 add string
	m_ComboxV15.AddString("0x00");
	m_ComboxV15.AddString("0x01");
	m_ComboxV15.AddString("0x02");
	m_ComboxV15.AddString("0x03");
	m_ComboxV15.AddString("0x04");
	m_ComboxV15.AddString("0x05");
	m_ComboxV15.AddString("0x06");
	m_ComboxV15.AddString("0x07");
	m_ComboxV15.AddString("0x08");
	m_ComboxV15.AddString("0x09");
	m_ComboxV15.AddString("0x0a");
	m_ComboxV15.AddString("0x0b");
	m_ComboxV15.AddString("0x0c");
	m_ComboxV15.AddString("0x0d");
	m_ComboxV15.AddString("0x0e");
	m_ComboxV15.AddString("0x0f");
	m_ComboxV15.SetCurSel(8);

	//Ipix add string
	m_ComboxIpix.AddString("0x00");
	m_ComboxIpix.AddString("0x01");
	m_ComboxIpix.AddString("0x02");
	m_ComboxIpix.AddString("0x03");
	m_ComboxIpix.AddString("0x04");
	m_ComboxIpix.AddString("0x05");
	m_ComboxIpix.AddString("0x06");
	m_ComboxIpix.AddString("0x07");
	m_ComboxIpix.AddString("0x08");
	m_ComboxIpix.AddString("0x09");
	m_ComboxIpix.AddString("0x0a");
	m_ComboxIpix.AddString("0x0b");
	m_ComboxIpix.AddString("0x0c");
	m_ComboxIpix.AddString("0x0d");
	m_ComboxIpix.AddString("0x0e");
	m_ComboxIpix.AddString("0x0f");
	m_ComboxIpix.SetCurSel(8);

	//Switch add string
	m_ComboxSwitch.AddString("0 high gain");
	m_ComboxSwitch.AddString("1 low gain");
	m_ComboxSwitch.SetCurSel(0);

	//TX add string
	m_ComboxTX.AddString("0x00");
	m_ComboxTX.AddString("0x01");
	m_ComboxTX.AddString("0x02");
	m_ComboxTX.AddString("0x03");
	m_ComboxTX.AddString("0x04");
	m_ComboxTX.AddString("0x05");
	m_ComboxTX.AddString("0x06");
	m_ComboxTX.AddString("0x07");
	m_ComboxTX.AddString("0x08");
	m_ComboxTX.AddString("0x09");
	m_ComboxTX.AddString("0x0a");
	m_ComboxTX.AddString("0x0b");
	m_ComboxTX.AddString("0x0c");
	m_ComboxTX.AddString("0x0d");
	m_ComboxTX.AddString("0x0e");
	m_ComboxTX.AddString("0x0f");
	m_ComboxTX.SetCurSel(15);

	m_ComboxTX2.AddString("0x00");
	m_ComboxTX2.AddString("0x01");
	m_ComboxTX2.AddString("0x02");
	m_ComboxTX2.AddString("0x03");
	m_ComboxTX2.AddString("0x04");
	m_ComboxTX2.AddString("0x05");
	m_ComboxTX2.AddString("0x06");
	m_ComboxTX2.AddString("0x07");
	m_ComboxTX2.AddString("0x08");
	m_ComboxTX2.AddString("0x09");
	m_ComboxTX2.AddString("0x0a");
	m_ComboxTX2.AddString("0x0b");
	m_ComboxTX2.AddString("0x0c");
	m_ComboxTX2.AddString("0x0d");
	m_ComboxTX2.AddString("0x0e");
	m_ComboxTX2.AddString("0x0f");
	m_ComboxTX2.SetCurSel(15);

	m_ComboxTX3.AddString("0x00");
	m_ComboxTX3.AddString("0x01");
	m_ComboxTX3.AddString("0x02");
	m_ComboxTX3.AddString("0x03");
	m_ComboxTX3.AddString("0x04");
	m_ComboxTX3.AddString("0x05");
	m_ComboxTX3.AddString("0x06");
	m_ComboxTX3.AddString("0x07");
	m_ComboxTX3.AddString("0x08");
	m_ComboxTX3.AddString("0x09");
	m_ComboxTX3.AddString("0x0a");
	m_ComboxTX3.AddString("0x0b");
	m_ComboxTX3.AddString("0x0c");
	m_ComboxTX3.AddString("0x0d");
	m_ComboxTX3.AddString("0x0e");
	m_ComboxTX3.AddString("0x0f");
	m_ComboxTX3.SetCurSel(15);

	m_ComboxTX4.AddString("0x00");
	m_ComboxTX4.AddString("0x01");
	m_ComboxTX4.AddString("0x02");
	m_ComboxTX4.AddString("0x03");
	m_ComboxTX4.AddString("0x04");
	m_ComboxTX4.AddString("0x05");
	m_ComboxTX4.AddString("0x06");
	m_ComboxTX4.AddString("0x07");
	m_ComboxTX4.AddString("0x08");
	m_ComboxTX4.AddString("0x09");
	m_ComboxTX4.AddString("0x0a");
	m_ComboxTX4.AddString("0x0b");
	m_ComboxTX4.AddString("0x0c");
	m_ComboxTX4.AddString("0x0d");
	m_ComboxTX4.AddString("0x0e");
	m_ComboxTX4.AddString("0x0f");
	m_ComboxTX4.SetCurSel(15);

	//Amux add string
	m_ComboxAmux.AddString("Vbgr");
	m_ComboxAmux.AddString("Vcm");
	m_ComboxAmux.AddString("V24");
	m_ComboxAmux.AddString("V15");
	m_ComboxAmux.AddString("V20");
	m_ComboxAmux.SetCurSel(0);

	//Test ADC add string
	m_ComboxTestADC.AddString("TestADC");
	m_ComboxTestADC.AddString("None");
	m_ComboxTestADC.SetCurSel(0);

	((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK1))->SetCheck(true);
	// ((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK2))->SetCheck(true);
	// ((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK3))->SetCheck(true);

	// dta position file is parsed at GraDlg Init. Need to make sure GraDlg Init is run first.
	if (numChannels < 4) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK4))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime4))->EnableWindow(false);
	}
	if (numChannels < 3) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK3))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime3))->EnableWindow(false);
	}
	// get trim data from extern trim file

	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime, sIntTime1);
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime2, sIntTime2);
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime3, sIntTime3);
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime4, sIntTime4);

	CString ostr;
	ostr.Format("%0.3f", m_tc95);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen95, ostr);
	ostr.Format("%0.3f", m_tc55);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen55, ostr);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ee_continue = true;

LRESULT CTrimDlg::OnTrimProcess(WPARAM wParam, LPARAM lParam)
{
	BYTE type;
	type = RxData[4];	//´Ó½ÓÊÕÊý¾ÝÖÐÈ¡³ötype·ÖÖ§

	CString sTemp1,sTemp2;
	sTemp1.Empty();
	sTemp2.Empty();

	CString sTem;
	sTem.Empty();
	unsigned char cTem[4];
	float * fTem;

	switch(type)
	{
	case 0x11:		// PCR temp1
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTemp1 = sTem;
			SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP1,sTemp1);
			//Çå³ýÊý¾Ý´«Êäbuffer
			memset(RxData,0,sizeof(RxData));
			break;
		}
	case 0x12:		// PCR temp2
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTemp2 = sTem;
			SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP2,sTemp2);
			//Çå³ýÊý¾Ý´«Êäbuffer
			memset(RxData,0,sizeof(RxData));
			break;
		}
	case 0x2d:		// EEPROM data, check parity here too.
	{
		BYTE eeprom_parity = 0;
		int index = RxData[7];		// For command type 2d EEPROM read command
		int npages = RxData[6];
		bool parity_ok = true;

		for (int i = 0; i < EPKT_SZ + 1; i++) {		// 
			EepromBuff[index][i] = RxData[8 + i];

			if (i < EPKT_SZ) {
				eeprom_parity += RxData[8 + i];
			}
			else {
				if (eeprom_parity != RxData[8 + i]) {
					MessageBox(_T("Packet parity error!"));
					parity_ok = false;
				}
			}
		}

		//=======print data============

		CString str, pstr;

		if (parity_ok) {
			pstr = " Parity OK.";
		}
		else {
			pstr = " Parity fail";
		}

		str.Format("eeprom read data: %d", index);
		g_pGraDlg->DisplayPollStatus(str);
		for (int i = 0; i < EPKT_SZ; i++) {
			str.Format("%0.2x", RxData[8 + i]);
//			g_pGraDlg->DisplayPollStatus(str);
		}
		str.Format("%0.2x(P)", RxData[8 + EPKT_SZ]);
		g_pGraDlg->DisplayPollStatus(str + pstr);


		//=============================

		if (index < npages - 1) ee_continue = true;
		else ee_continue = false;

		memset(RxData, 0, sizeof(RxData));
		break;
	}
	default: 
		break;
	}
	return 0;
}

void CTrimDlg::ResetTrim(void)
{
	//	assert(ReceiveTemCycNum > 0);

	//	if (ReceiveTemCycNum > 0) {		// cycler running
	//		return;
	//	}

	ASSERT(ReceiveTemCycNum == 0);		// USB reattach when cycler running

										// Need to cycle through all chips.

										//	SelSensor(1);
	ResetParams();
	//	SelSensor(2);
	//	ResetParams();
	//	SelSensor(3);
	//	ResetParams();
	//	SelSensor(4);
	//	ResetParams();

	// init GUI
	m_ComboxV20.SetCurSel(8);
	m_ComboxV15.SetCurSel(8);
	//	m_ComboxRampgen.SetCurSel((int)rg_trim);
	m_ComboxRange.SetCurSel(0x0f);
	m_ComboxV24.SetCurSel(8);
	m_ComboxIpix.SetCurSel(8);
	m_ComboxTX.SetCurSel(0xf);
	m_ComboxTX2.SetCurSel(0xf);
	m_ComboxTX3.SetCurSel(0xf);
	m_ComboxTX4.SetCurSel(0xf);
	m_ComboxSwitch.SetCurSel(0);
	m_ComboxAmux.SetCurSel(0);
	m_ComboxTestADC.SetCurSel(0);

	// Init trim values
	if (g_pTrimReader) {
		SelSensor(1);
		SetRampgen((char)g_pTrimReader->Node[0].rampgen);
		SetTXbin(0xf);
		SetRange((char)g_pTrimReader->Node[0].range);
		SetV15(g_pTrimReader->Node[0].auto_v15);

		SelSensor(2);
		SetRampgen((char)g_pTrimReader->Node[1].rampgen);
		SetTXbin(0xf);
		SetRange((char)g_pTrimReader->Node[1].range);
		SetV15(g_pTrimReader->Node[1].auto_v15);

		SelSensor(3);
		SetRampgen((char)g_pTrimReader->Node[2].rampgen);
		SetTXbin(0xf);
		SetRange((char)g_pTrimReader->Node[2].range);
		SetV15(g_pTrimReader->Node[2].auto_v15);

		SelSensor(4);
		SetRampgen((char)g_pTrimReader->Node[3].rampgen);
		SetTXbin(0xf);
		SetRange((char)g_pTrimReader->Node[3].range);
		SetV15(g_pTrimReader->Node[3].auto_v15);
	}

	gain_mode = 0;			// initialize to high gain mode, consistent with HW default

	if (g_pTrimReader) {
		SetV20(g_pTrimReader->Node[0].auto_v20[1], 1);
		SetV20(g_pTrimReader->Node[1].auto_v20[1], 2);
		SetV20(g_pTrimReader->Node[2].auto_v20[1], 3);
		SetV20(g_pTrimReader->Node[3].auto_v20[1], 4);
	}

	int_time1 = int_time2 = int_time3 = int_time4 = 1;		// int time has been reset at this point.

															//=======================================

	SetLEDConfig(1, 1, 1, 1, 1);			// Set Multi LED mode, first enable all channels, then disable all channels.
	Sleep(100);								// Why do we need to do this
	SetLEDConfig(1, 0, 0, 0, 0);

	if (ReceiveTemCycNum == 0) return;

	if (gain_mode != !m_GainMode) {					// if cycler in progress, restore GUI values.

		gain_mode = !m_GainMode;
		int gain = gain_mode; // Initialize gain mode

		SetGainMode(gain, 1);
		if (!gain) SetV20(g_pTrimReader->Node[0].auto_v20[1], 1);
		else SetV20(g_pTrimReader->Node[0].auto_v20[0], 1);

		SetGainMode(gain, 2);
		if (!gain) SetV20(g_pTrimReader->Node[1].auto_v20[1], 2);
		else SetV20(g_pTrimReader->Node[1].auto_v20[0], 2);

		SetGainMode(gain, 3);
		if (!gain) SetV20(g_pTrimReader->Node[2].auto_v20[1], 3);
		else SetV20(g_pTrimReader->Node[2].auto_v20[0], 3);

		SetGainMode(gain, 4);
		if (!gain) SetV20(g_pTrimReader->Node[3].auto_v20[1], 4);
		else SetV20(g_pTrimReader->Node[3].auto_v20[0], 4);
	}

	// Set integration time according to GUI value.

	float fn = 0.0f;

	fn = (float)atof(sIntTime1);
	SetIntTime(cur_factor1 * fn, 1);
	fn = (float)atof(sIntTime2);
	SetIntTime(cur_factor2 * fn, 2);
	fn = (float)atof(sIntTime3);
	SetIntTime(cur_factor3 * fn, 3);
	fn = (float)atof(sIntTime4);
	SetIntTime(cur_factor4 * fn, 4);
}

void CTrimDlg::OnCbnSelchangeComboRampgen()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRampgen.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxRampgen.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);	//½«»ñÈ¡µÄ×Ö·û´®×ª³É16½øÖÆ×Ö·û

	SetRampgen(dataEdit[0]);
}


void CTrimDlg::OnCbnSelchangeComboRange()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRange.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxRange.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	SetRange(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV24()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV24.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxV24.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboV20()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV20.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxV20.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);

	SetV20(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV15()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxV15.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxV15.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x05;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboIpix()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxIpix.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxIpix.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x06;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboSwitch()
{
	// TODO: Add your control notification handler code here

	TrimFlag = SENDTRIMMSG;

	int nIndex = m_ComboxSwitch.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxSwitch.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ

	if (strText == "0 high gain")
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
		TrimCalMainMsg();
		gain_mode = 0;
	}
	else	
	{	
		if (strText == "1 low gain")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
			gain_mode = 1;
		}
		else
			MessageBox("please check the right item");
	}	
}


void CTrimDlg::OnCbnSelchangeComboTxbin()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}


	int nIndex = m_ComboxTX.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxTX.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(1);

	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboAmuxsel()
{
	// TODO: Add your control notification handler code here

	TrimFlag = SENDTRIMMSG;

	int nIndex = m_ComboxAmux.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxAmux.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ
	//strTextÊÇbuffer


	if (strText == "Vbgr")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x09;		//data type, date edit first byte
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
		TrimCalMainMsg();
	}	
	else
	{
		if (strText == "Vcm")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x09;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
		}
		else
		{
			if (strText == "V24")
			{
				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x01;		//command
				TxData[2] = 0x02;		//data length
				TxData[3] = 0x09;		//data type, date edit first byte
				TxData[4] = 0x02;		//real data, date edit second byte
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
				TrimCalMainMsg();
			}
			else
			{
				if (strText == "V15")
				{
					TxData[0] = 0xaa;		//preamble code
					TxData[1] = 0x01;		//command
					TxData[2] = 0x02;		//data length
					TxData[3] = 0x09;		//data type, date edit first byte
					TxData[4] = 0x03;		//real data, date edit second byte
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
					TrimCalMainMsg();
				}
				else
				{
					if (strText == "V20")
					{
						TxData[0] = 0xaa;		//preamble code
						TxData[1] = 0x01;		//command
						TxData[2] = 0x02;		//data length
						TxData[3] = 0x09;		//data type, date edit first byte
						TxData[4] = 0x04;		//real data, date edit second byte
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
						TrimCalMainMsg();
					}
					else
						MessageBox("please check right item");
				}
			}
		}

	}
}


void CTrimDlg::OnCbnSelchangeComboTestadc()
{
	// TODO: Add your control notification handler code here

	TrimFlag = SENDTRIMMSG;

	int nIndex = m_ComboxTestADC.GetCurSel();		//»ñÈ¡comboxÖÐ±»Ñ¡ÖÐitemµÄÐÐºÅ

	CString strText;
	m_ComboxTestADC.GetLBText(nIndex,strText);		//½«Ñ¡ÖÐµÄitemµÄÄÚÈÝÒÔ×Ö·û´®µÄ¸ñÊ½·ÅÈëbufferÖÐ


	if (strText == "TestADC")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x0a;		//data type, date edit first byte
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
		TrimCalMainMsg();
	}
	else	
	{	
		if (strText == "None")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x0a;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
		}
		else
			MessageBox("please check the right item");
	}
}


void CTrimDlg::OnBnClickedBtnSendcmd()
{
	// TODO: Add your control notification handler code here

	CString sData, sCmd, sType;
	char* cData ="";
	char* cCmd = "";
	char* cType = "";
	int d_Num;

	TrimFlag = SENDTRIMMSG;

	memset(TxData,0,sizeof(TxData));

	GetDlgItemText(IDC_EDIT_CMD,sCmd);
	d_Num = (sCmd.GetLength())/2;
	cCmd = EditDataCvtChar(sCmd,cCmd);

	GetDlgItemText(IDC_EDIT_TYPE,sType);
	d_Num = (sType.GetLength())/2;
	cType = EditDataCvtChar(sType,cType);

	GetDlgItemText(IDC_EDIT_Data,sData);
	d_Num = (sData.GetLength())/2;

	cData = EditDataCvtChar(sData,cData);

	TxData[0] = 0xaa;
	TxData[1]= cCmd[0];
	TxData[2]= d_Num+1;
	TxData[3]= cType[0];
	for(int i=0; i<d_Num; i++)
	{
		TxData[i+4] = cData[i];
	}
	for(int j=1; j<(d_Num+4); j++)
	{
		TxData[d_Num+4] += TxData[j];
	}
	if (TxData[d_Num+4] == 0x17)
		TxData[d_Num+4] = 0x18;
	else
		TxData[d_Num+4] = TxData[d_Num+4];
	TxData[d_Num+5] = 0x17;
	TxData[d_Num+6] = 0x17;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::SetGainMode(int gain)	// gain - 1: low gain; 0: high gain
{
	TrimFlag = SENDTRIMMSG;

	if (!gain)	// high gain
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
		TrimCalMainMsg();
		gain_mode = 0;
	}
	else if(gain)	// low gain
	{	
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
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
			TrimCalMainMsg();
			gain_mode = 1;
	}	
}

void CTrimDlg::SetGainMode(int gain, char i)	// gain - 0: low gain; 1: high gain
{
	SelSensor(i);
	SetGainMode(gain);
}


// replicate the above 3 more times


void CTrimDlg::SetIntTime(float kfl, int ch)
{
	SelSensor(ch);

	if (kfl < 1) kfl = 1;
	else if (kfl > 10000) kfl = 10000;	// max 10,000 ms for qPCR Alpha

	switch (ch) {
	case 1:
		int_time1 = kfl;
		break;

	case 2:
		int_time2 = kfl;
		break;

	case 3:
		int_time3 = kfl;
		break;

	case 4:
		int_time4 = kfl;
		break;

	default:
		break;
	}

	unsigned char * hData = (unsigned char *) & kfl;	//½«¸¡µãÊý¾Ý×ª»¯ÎªÊ®Áù½øÖÆÊý¾Ý

	TrimBuf[0] = hData[0];	//´æµ½´°¿Ú´«µÝbuffer
	TrimBuf[1] = hData[1];
	TrimBuf[2] = hData[2];
	TrimBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x20;		//data type, date edit first byte
	TxData[4] = TrimBuf[0];		//real data, date edit second byte
	TxData[5] = TrimBuf[1];
	TxData[6] = TrimBuf[2];
	TxData[7] = TrimBuf[3];
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[8] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7];		//check sum
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[9] = 0x17;		//back code
	TxData[10] = 0x17;		//back code

	//Call maindlg message
	TrimFlag = TRIM_TWO_HID;
	TrimCalMainMsg();
}

void CTrimDlg::SetRange(char range)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = range;	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::SetV20(char v20)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte
	TxData[4] = v20;		//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetV15(char v15)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x05;		//data type, date edit first byte
	TxData[4] = v15;	//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SelSensor(BYTE i)
{
	// TODO: Add your control notification handler code here

	if(i < 1 || i > 4) return;

	TrimFlag = SENDTRIMMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = i - 1;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetV20(char v20, char i)
{
	SelSensor(i);
	SetV20(v20);
}

void CTrimDlg::SetRampgen(char rampgen)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = rampgen;	//real data, date edit second byte
	//0x01 means send video data
	//0x00 means stop video data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetTXbin(char txbin)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = txbin;	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::ResetTxBin(void)
{
		m_ComboxTX.SetCurSel(0x8);
		m_ComboxTX2.SetCurSel(0x8);
		m_ComboxTX3.SetCurSel(0x8);
		m_ComboxTX4.SetCurSel(0x8);
}


void CTrimDlg::OnBnClickedTrimBtnPcrtemp1()
{
	// TODO: Add your control notification handler code here
	TrimFlag = SENDTRIMMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x11;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedTrimBtnPcrtemp2()
{
	// TODO: Add your control notification handler code here
	TrimFlag = SENDTRIMMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x12;		//data type
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedBtnShowdata()
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_EDIT_REDATA, RegRecStr);
}


void CTrimDlg::OnClickedRadioTrimSensor1()
{
	// TODO: Add your control notification handler code here

	TrimFlag = TRIM_TWO_HID;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedRadioTrimSensor2()
{
	// TODO: Add your control notification handler code here

	TrimFlag = TRIM_TWO_HID;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedRadioTrimSensor3()
{
	// TODO: Add your control notification handler code here
	TrimFlag = TRIM_TWO_HID;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x02;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedRadioTrimSensor4()
{
	// TODO: Add your control notification handler code here

	TrimFlag = TRIM_TWO_HID;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x03;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedBtnItgtim()
{
	// TODO: Add your control notification handler code here

	CString kstg;
	kstg.Empty();
	float kfl = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime,kstg);
	kfl = (float)atof(kstg);	//½«×Ö·û´®×ª³É¸¡µãÐÍÊý¾Ý

	SetIntTime(kfl, 1);
}



void CTrimDlg::OnBnClickedBtnItgtim2()
{
	// TODO: Add your control notification handler code here

	CString kstg;
	kstg.Empty();
	float kfl = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime2,kstg);
	kfl = (float)atof(kstg);	//½«×Ö·û´®×ª³É¸¡µãÐÍÊý¾Ý

	SetIntTime(kfl, 2);
}


void CTrimDlg::OnBnClickedBtnItgtim3()
{
	// TODO: Add your control notification handler code here

	CString kstg;
	kstg.Empty();
	float kfl = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime3,kstg);
	kfl = (float)atof(kstg);	//½«×Ö·û´®×ª³É¸¡µãÐÍÊý¾Ý

	SetIntTime(kfl, 3);
}


void CTrimDlg::OnBnClickedBtnItgtim4()
{
	// TODO: Add your control notification handler code here

	CString kstg;
	kstg.Empty();
	float kfl = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime4,kstg);
	kfl = (float)atof(kstg);	//½«×Ö·û´®×ª³É¸¡µãÐÍÊý¾Ý

	SetIntTime(kfl, 4);
}


void CTrimDlg::OnCbnSelchangeComboTxbin2()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxTX2.GetCurSel();		//

	CString strText;
	m_ComboxTX2.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(2);

	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboTxbin3()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxTX3.GetCurSel();		//

	CString strText;
	m_ComboxTX3.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(3);

	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboTxbin4()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxTX4.GetCurSel();		//

	CString strText;
	m_ComboxTX4.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(4);

	SetTXbin(dataEdit[1]);
}

void CTrimDlg::SetLEDConfig(BOOL IndvEn, BOOL Chan1, BOOL Chan2, BOOL Chan3, BOOL Chan4)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x23;		//data type, date edit first byte

	if (!IndvEn)
	{
		TxData[4] = Chan1 ? 1 : 0;									//real data, date edit second byte
	}
	else
	{
		TxData[4] = 0x80;
		if (Chan1)
			TxData[4] |= 1;
		if (Chan2)
			TxData[4] |= 2;
		if (Chan3)
			TxData[4] |= 4;
		if (Chan4)
			TxData[4] |= 8;
	}

	TxData[5] = TxData[1] + TxData[2] + TxData[3] + TxData[4];		//check sum
	
	if (TxData[5] == 0x17)
		TxData[5] = 0x18;
	else
		TxData[5] = TxData[5];
	
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = SENDTRIMMSG;
	TrimCalMainMsg();
}

void CTrimDlg::TrimGetPCRMaskStatus()
{
	int ck1, ck2, ck3, ck4;

	if ((BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TRIM_MASK1)))
		ck1 = 1;
	else
		ck1 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TRIM_MASK2))
		ck2 = 1;
	else
		ck2 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TRIM_MASK3))
		ck3 = 1;
	else
		ck3 = 0;
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK_TRIM_MASK4))
		ck4 = 1;
	else
		ck4 = 0;

	PCRMask = (ck4 << 3) | (ck3 << 2) | (ck2 << 1) | ck1;
}

void CTrimDlg::SetPCRMask(int mask)
{
	CButton *bt1, *bt2, *bt3, *bt4;

	bt1 = (CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK1);
	bt2 = (CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK2);
	bt3 = (CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK3);
	bt4 = (CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK4);

	if (mask & 0x1) {
		bt1->SetCheck(true);
	}
	else {
		bt1->SetCheck(false);
	}

	if (mask & 0x2) {
		bt2->SetCheck(true);
	}
	else {
		bt2->SetCheck(false);
	}

	if (mask & 0x4) {
		bt3->SetCheck(true);
	}
	else {
		bt3->SetCheck(false);
	}

	if (mask & 0x8) {
		bt4->SetCheck(true);
	}
	else {
		bt4->SetCheck(false);
	}
}

void CTrimDlg::CommitTrim()
{
	// Set gain mode if GUI gain mode difference from HW gain mode.

	if (gain_mode != !m_GainMode) {

		gain_mode = !m_GainMode;
		int gain = gain_mode; // Initialize gain mode

		SetGainMode(gain, 1);
		if (!gain) SetV20(g_pTrimReader->Node[0].auto_v20[1], 1);
		else SetV20(g_pTrimReader->Node[0].auto_v20[0], 1);

		SetGainMode(gain, 2);
		if (!gain) SetV20(g_pTrimReader->Node[1].auto_v20[1], 2);
		else SetV20(g_pTrimReader->Node[1].auto_v20[0], 2);

		SetGainMode(gain, 3);
		if (!gain) SetV20(g_pTrimReader->Node[2].auto_v20[1], 3);
		else SetV20(g_pTrimReader->Node[2].auto_v20[0], 3);

		SetGainMode(gain, 4);
		if (!gain) SetV20(g_pTrimReader->Node[3].auto_v20[1], 4);
		else SetV20(g_pTrimReader->Node[3].auto_v20[0], 4);
	}

	// Set integration time according to GUI value.

	float epsilon = 0.01f;
	float fn = 0.0f;

	fn = (float)atof(sIntTime1);
	if (abs(int_time1 - fn) > epsilon) {
		SetIntTime(fn, 1);
	}

	fn = (float)atof(sIntTime2);
	if (abs(int_time2 - fn) > epsilon) {
		SetIntTime(fn, 2);
	}

	fn = (float)atof(sIntTime3);
	if (abs(int_time3 - fn) > epsilon) {
		SetIntTime(fn, 3);
	}

	fn = (float)atof(sIntTime4);
	if (abs(int_time4 - fn) > epsilon) {
		SetIntTime(fn, 4);
	}
}

float CTrimDlg::DynamicUpdateIntTime(float factor, int chan)
{
	//	float fn = 1.0f;

	switch (chan) {
	case 0:
		int_time1 = (float)atof(sIntTime1);
		int_time1 = round(int_time1 * factor);
		SetIntTime(int_time1, 1);
		cur_factor1 = int_time1 / (float)atof(sIntTime1);
		return cur_factor1;
		// break;

	case 1:
		int_time2 = (float)atof(sIntTime2);
		int_time2 = round(int_time2 * factor);
		SetIntTime(int_time2, 2);
		cur_factor2 = int_time2 / (float)atof(sIntTime2);
		return cur_factor2;
		//		break;

	case 2:
		int_time3 = (float)atof(sIntTime3);
		int_time3 = round(int_time3 * factor);
		SetIntTime(int_time3, 3);
		cur_factor3 = int_time3 / (float)atof(sIntTime3);
		return cur_factor3;
		//		break;

	case 3:
		int_time4 = (float)atof(sIntTime4);
		int_time4 = round(int_time4 * factor);
		SetIntTime(int_time4, 4);
		cur_factor4 = int_time4 / (float)atof(sIntTime4);
		return cur_factor4;
		//		break;

	default:
		return 1;
	}
}


void CTrimDlg::OnBnClickedRadioLowgain()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CTrimDlg::OnBnClickedRadioHighGain()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CTrimDlg::OnBnClickedRadioFrame12()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	SetFrameSizeFromGUI();
}


void CTrimDlg::OnBnClickedRadioFrame24()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	SetFrameSizeFromGUI();
}

void CTrimDlg::SetFrameSizeFromGUI()
{
	frame_size = m_FrameSize;

	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}
	
	TrimFlag = SENDTRIMMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x25;		//data type
	TxData[4] = (BYTE)m_FrameSize;
	TxData[5] = 0x00;
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	for (int i = 1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8] == 0x17)
		TxData[8] = 0x18;
	else
		TxData[8] = TxData[8];
	TxData[9] = 0x17;
	TxData[10] = 0x17;

	//Send message to main dialog
	TrimCalMainMsg();		//
}


void CTrimDlg::ResetParams()
{
	TrimFlag = SENDTRIMMSG;					// Zhimin to do: Need to reset each channel

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x0F;		//data type, dat edit first byte
	TxData[4] = 0x00;		//real data, data edit second byte
	TxData[5] = 0x00;		//real data, data edit third byte
	TxData[6] = 0x13;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

extern CString sTemFirmwareVer;

void CTrimDlg::OnClickedTrimBtnTest()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	TrimFlag = SENDTRIMMSG;					// Zhimin to do: Need to reset each channel

	CDialog *pDlg = new CDialog;
	pDlg->Create(IDD_VERSION_DIALOG);

	// read HID firmware version
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x27;		//data type
	TxData[4] = 0x00;
	TxData[5] = 0x00;
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	for (int i = 1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8] == 0x17)
		TxData[8] = 0x18;
	else
		TxData[8] = TxData[8];
	TxData[9] = 0x17;
	TxData[10] = 0x17;

	//Call maindlg message
	TrimCalMainMsg();

	pDlg->SetDlgItemText(IDC_VERDLG_STATIC_TEMHIDVER, sTemFirmwareVer);
	pDlg->ShowWindow(SW_NORMAL);
}

void CTrimDlg::OnBnClickedTrimBtnSn()
{
	// TODO: Add your control notification handler code here

	CString str, s;
	int sn1 = (int)g_pDPReader->serial_number1;
	int sn2 = (int)g_pDPReader->serial_number2;
	s = g_pDPReader->id_str.c_str();

	if (g_pDPReader->version == 0) {
		s = (char)g_pDPReader->id;

		str.Format("%s-%d-%d", s, sn1, sn2);
	}
	else {
		// str = m_DPReader.id_str.c_str();
		str.Format("%s-%d-%d", s, sn1, sn2);
	}


	CString SN;

	SN = "123456";

	SerialNumDlg snDlg;

	snDlg.m_SN = g_pDPReader->id_str.c_str();
	
	if (snDlg.DoModal() == IDOK) {
		SN = snDlg.m_SN;

		g_pDPReader->id_str = SN;

		SN = "";
	}
}



void CTrimDlg::OnKillfocusTrimEditIntergratetime()
{
	CString str;
	str.Empty();
	float fn = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime, str);
	fn = (float)atof(str);	

	if (fn > 65000 || fn < 1) {
		MessageBox("Please set value between 1 and 65000");
	}
	else {
		sIntTime1 = str;
	}

	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime, sIntTime1);
}


void CTrimDlg::OnKillfocusTrimEditIntergratetime2()
{
	CString str;
	str.Empty();
	float fn = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime2, str);
	fn = (float)atof(str);

	if (fn > 65000 || fn < 1) {
		MessageBox("Please set value between 1 and 65000");
	}
	else {
		sIntTime2 = str;
	}

	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime2, sIntTime2);
}


void CTrimDlg::OnKillfocusTrimEditIntergratetime3()
{
	CString str;
	str.Empty();
	float fn = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime3, str);
	fn = (float)atof(str);

	if (fn > 65000 || fn < 1) {
		MessageBox("Please set value between 1 and 65000");
	}
	else {
		sIntTime3 = str;
	}

	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime3, sIntTime3);
}


void CTrimDlg::OnKillfocusTrimEditIntergratetime4()
{
	CString str;
	str.Empty();
	float fn = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime4, str);
	fn = (float)atof(str);

	if (fn > 65000 || fn < 1) {
		MessageBox("Please set value between 1 and 65000");
	}
	else {
		sIntTime4 = str;
	}

	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime4, sIntTime4);
}

void CTrimDlg::OnEnKillfocusTrimEditTempcompen95()
{
	// TODO: Add your control notification handler code here

	CString str, ostr;
	ostr.Format("%0.3f", m_tc95);

	float t;
	GetDlgItemText(IDC_TRIM_EDIT_TempCompen95, str);

	t = (float)atof(str);

	if (t > 2 || t < -2) {
		MessageBox("Please set value between -2 and 2");
		SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen95, ostr);	
	}
	else {
		m_tc95 = t;
	}
}


void CTrimDlg::OnEnKillfocusTrimEditTempcompen55()
{
	// TODO: Add your control notification handler code here

	CString str, ostr;
	ostr.Format("%0.3f", m_tc55);

	float t;
	GetDlgItemText(IDC_TRIM_EDIT_TempCompen55, str);

	t = (float)atof(str);

	if (t > 2 || t < -2) {
		MessageBox("Please set value between -2 and 2");
		SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen55, ostr);
	}
	else {
		m_tc55 = t;
	}
}


// Useful for load Json value into GUI

void CTrimDlg::SetIntTimeEdit1(CString str)
{
	sIntTime1 = str;
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime, str);
}

void CTrimDlg::SetIntTimeEdit2(CString str)
{
	sIntTime2 = str;
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime2, str);
}

void CTrimDlg::SetIntTimeEdit3(CString str)
{
	sIntTime3 = str;
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime3, str);
}

void CTrimDlg::SetIntTimeEdit4(CString str)
{
	sIntTime4 = str;
	SetDlgItemTextA(IDC_TRIM_EDIT_IntergrateTime4, str);
}

void CTrimDlg::SetIntTimeGUI(CString str, int ch)
{
	switch (ch) {
	case 1:
		SetIntTimeEdit1(str);
		break;

	case 2:
		SetIntTimeEdit2(str);
		break;

	case 3:
		SetIntTimeEdit3(str);
		break;

	case 4:
		SetIntTimeEdit4(str);
		break;

	default:
		break;
	}
}

void CTrimDlg::SetMGainMode(int g)
{
	CButton *lg = (CButton*)GetDlgItem(IDC_RADIO_LOWGAIN);
	CButton *hg = (CButton*)GetDlgItem(IDC_RADIO_HIGHGAIN);

	m_GainMode = g;
	
	if (!g) {
		lg->SetCheck(true);
		hg->SetCheck(false);
	}
	else {
		hg->SetCheck(true);
		lg->SetCheck(false);
	}
}


//=============== EEPROM write =======================

// Use EPKT_SZ, NUM_EPKT

BYTE channelFormat = 0;

void CTrimDlg::EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size)
{
	if (packet_size > 53) return;		// Do nothing if packet size too big.

										/*	Total HID packet is 64, minus 6 (header, tail) is 58,
										-4 protocol header, 54, minus 1 parity, data packet max size is 53.
										*/

	BYTE eeprom_parity = 0;

	TrimFlag = SENDTRIMMSG;

	int len = packet_size + 4 + 1;		// 4: protocol header, Add a EEPROM parity byte
	BYTE check_sum = 0;
	int i;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = len;		//data length
	TxData[3] = 0x2d;		//data type, date edit first byte
	TxData[4] = (BYTE)chan;		//real data, date edit second byte
	TxData[5] = (BYTE)total_packets;
	TxData[6] = (BYTE)packet_index;

	for (i = 0; i < packet_size; i++) {
		TxData[7 + i] = packet_data[i];
		eeprom_parity += packet_data[i];
	}

	TxData[7 + packet_size] = eeprom_parity;

	for (i = 1; i < 8 + packet_size; i++) {
		check_sum += TxData[i];
	}

	TxData[8 + packet_size] = check_sum;		//check sum

	if (TxData[8 + packet_size] == 0x17) {
		TxData[8 + packet_size] = 0x18;
	}

	TxData[9 + packet_size] = 0x17;				//back code
	TxData[10 + packet_size] = 0x17;			//back code

//=======print data============

	CString str;

	str.Format("eeprom write data: %d", packet_index);
	g_pGraDlg->DisplayPollStatus(str);
	for (i = 0; i < packet_size; i++) {
		str.Format("%0.2x", packet_data[i]);
//		g_pGraDlg->DisplayPollStatus(str);
	}
	str.Format("%0.2x(P)", eeprom_parity);
	g_pGraDlg->DisplayPollStatus(str);

//=============================

	TrimCalMainMsg();							//Call maindlg message
}

void CTrimDlg::EEPROMRead(int chan)
{
	TrimFlag = EEPROMMSG;			// only the eeprom read use this message type, need to read multi packets

	TxData[0] = 0xaa;					//preamble code
	TxData[1] = 0x04;					//command
	TxData[2] = 0x02;					//data length
	TxData[3] = 0x2d;					//data type
	TxData[4] = (BYTE)chan;			//	real data
									//	TxData[5] = 0x00;
	TxData[5] = TxData[1] + TxData[2] + TxData[3] + TxData[4];		//check sum
	if (TxData[5] == 0x17)
		TxData[5] = 0x18;
	else
		TxData[5] = TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimCalMainMsg();		//Call maindlg message
}

// Repurposed to erase flash

void CTrimDlg::OnBnClickedTrimBtnLoad()
{
	if (!g_DeviceDetected) {
		MessageBox("PDx16 Device Not Attached");
		return;
	}

	TrimFlag = SENDTRIMMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x04;		//data length
	TxData[3] = 0x2d;		//data type, date edit first byte
	TxData[4] = 0x0;		//real data, date edit second byte
	TxData[5] = 0x1;
	TxData[6] = 0xe0;		// e0 means erase

	TxData[7] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5] + TxData[6];		//check sum
	if (TxData[7] == 0x17)
		TxData[7] = 0x18;

	TxData[8] = 0x17;		//back code
	TxData[9] = 0x17;		//back code

	TrimCalMainMsg();		//Call maindlg message


	if (RxData[1] == 0x0 && RxData[4] == 0x2d) {
		MessageBox("Flash Erased");
	}
}

void CTrimDlg::OnBnClickedButtonWreep()
{
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	BYTE	*pdata[4], *pdata0;			// copy of trim_buff by reference, 4 channels for trim data
	int		size, i, j, k;				// size of trim_buff
	int		npages, p_index, nchannels;

	g_pDPReader->tc95 = (BYTE)(m_tc95 * 64);
	g_pDPReader->tc55 = (BYTE)(m_tc55 * 64);

	//g_DPReader.Parse();		// Parsed in PCRProjDlg
	size = g_pDPReader->WriteTrimBuff();
	npages = (int)g_pDPReader->num_pages;
	nchannels = (int)g_pDPReader->num_channels;
	pdata0 = g_pDPReader->trim_buff;

	p_index = 0;

	for (i = 0; i < npages; i++) {
		EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata0 + EPKT_SZ * i, EPKT_SZ);
		p_index++;
	}

	// Write to EEPROM from trim buff

	for (k = 0; k < nchannels; k++) {
		if(g_pTrimReader->Node[k].version != 3) g_pTrimReader->Convert2Int(k);
		pdata[k] = g_pTrimReader->Node[k].trim_buff;
		size = g_pTrimReader->WriteTrimBuff(k);

		for (i = 0; i < NUM_EPKT; i++) {
			EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata[k] + EPKT_SZ * i, EPKT_SZ);
			p_index++;
		}
	}

	// Now read and verify

	EEPROMRead(0);

	int num_err = 0;

	for (i = 0; i < npages; i++) {
		for (j = 0; j < EPKT_SZ; j++) {
			if (pdata0[i * EPKT_SZ + j] != EepromBuff[i][j]) {
				num_err++;
			}
		}
	}

	for (k = 0; k < nchannels; k++) {
		for (i = 0; i < NUM_EPKT; i++) {
			for (j = 0; j < EPKT_SZ; j++) {
				if (pdata[k][i * EPKT_SZ + j] != EepromBuff[i + npages + k * NUM_EPKT][j]) {		// 4 packets per channel
					num_err++;
				}
			}
		}
	}

	if (num_err > 0) {
		CString str;
		str.Format("Flash program failed. err count: %d", num_err);
		MessageBox(str);
	}
	else {
		MessageBox("Flash program success!");
	}
}

CTrimReader m_TrimReader2;
DPReader m_DPReader2;

void CTrimDlg::OnBnClickedButtonRdeep()
{
	CString str("Flash read success!");
	
	str += ReadFlash();

	MessageBox(str);
}

#include "SystemConfig.cpp"

CString CTrimDlg::ReadFlash()
{
	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return "";
	}

	EEPROMRead(0);

	m_DPReader2.CopyEepromBuffAndRestore();			// Compare with with g_DPReader.

	int nchannels = (int)m_DPReader2.num_channels;
	int npages = (int)m_DPReader2.num_pages;
	int sn1 = m_DPReader2.serial_number1;
	int sn2 = m_DPReader2.serial_number2;
	BYTE ml = m_DPReader2.id;

	m_TrimReader2.NumNode = nchannels;

	for (int i = 0; i < nchannels; i++) {
		m_TrimReader2.CopyEepromBuff(i, npages + i * NUM_EPKT);
		m_TrimReader2.RestoreTrimBuff(i);
		m_TrimReader2.Node[i].version = 3;				// So it will use integer version KB matrix and FPN values
	}

	g_pTrimReader = &m_TrimReader2;
	g_pDPReader = &m_DPReader2;

	ResetTrim();

	// dta position file is parsed at GraDlg Init. Need to make sure GraDlg Init is run first.
	if (nchannels < 4) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK4))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime4))->EnableWindow(false);
	}
	if (nchannels < 3) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK3))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime3))->EnableWindow(false);
	}

	CString str, s;

	if (m_DPReader2.version == 0) {
		s = (char)ml;
		str.Format("%s-%d-%d", s, sn1, sn2);
	}
	else {
		str = m_DPReader2.id_str.c_str();
	}

//	MessageBox(str);

	m_tc95 = (float)g_pDPReader->tc95 / 64;
	m_tc55 = (float)g_pDPReader->tc55 / 64;

	CString ostr;
	ostr.Format("%0.3f", m_tc95);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen95, ostr);
	ostr.Format("%0.3f", m_tc55);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen55, ostr);

	return str;
}


void CTrimDlg::OnBnClickedButtonVerify()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	BYTE	*pdata[4], *pdata0;			// copy of trim_buff by reference, 4 channels for trim data
	int		size, i, j, k;				// size of trim_buff
	int		npages, p_index, nchannels;

	//g_DPReader.Parse();		// Parsed in PCRProjDlg
	size = g_pDPReader->WriteTrimBuff();
	npages = (int)g_pDPReader->num_pages;
	nchannels = (int)g_pDPReader->num_channels;
	pdata0 = g_pDPReader->trim_buff;

/*	p_index = 0;

	for (i = 0; i < npages; i++) {
		EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata0 + EPKT_SZ * i, EPKT_SZ);
		p_index++;
	}
*/
	// Write to EEPROM from trim buff

	for (k = 0; k < nchannels; k++) {
		g_pTrimReader->Convert2Int(k);
		pdata[k] = g_pTrimReader->Node[k].trim_buff;
		size = g_pTrimReader->WriteTrimBuff(k);

/*		for (i = 0; i < NUM_EPKT; i++) {
			EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata[k] + EPKT_SZ * i, EPKT_SZ);
			p_index++;
		}
*/	}

	// Now read and verify

	EEPROMRead(0);

	int num_err = 0;

	for (i = 0; i < npages; i++) {
		for (j = 0; j < EPKT_SZ; j++) {
			if (pdata0[i * EPKT_SZ + j] != EepromBuff[i][j]) {
				num_err++;
			}
		}
	}

	for (k = 0; k < nchannels; k++) {
		for (i = 0; i < NUM_EPKT; i++) {
			for (j = 0; j < EPKT_SZ; j++) {
				if (pdata[k][i * EPKT_SZ + j] != EepromBuff[i + npages + k * NUM_EPKT][j]) {		// 4 packets per channel
					num_err++;
				}
			}
		}
	}

	if (num_err > 0) {
		CString str;
		str.Format("Flash verify failed. err count: %d", num_err);
		MessageBox(str);
	}
	else {
		MessageBox("Flash verify success!");
	}
}


bool off_line_test = true;

extern ReportStrings test_report;


void CTrimDlg::OnBnClickedButtonSavef()
{
	// TODO: Add your control notification handler code here

	if (!g_DeviceDetected && !off_line_test) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	BYTE	*pdata[4], *pdata0, parity;			// copy of trim_buff by reference, 4 channels for trim data
	int		size, i, j, k;				// size of trim_buff
	int		npages, p_index, nchannels;

	// g_pDPReader->tc95 = (BYTE)(m_tc95 * 64);
	// g_pDPReader->tc55 = (BYTE)(m_tc55 * 64);

	//g_DPReader.Parse();		// Parsed in PCRProjDlg
	size = g_pDPReader->WriteTrimBuff();
	npages = (int)g_pDPReader->num_pages;
	nchannels = (int)g_pDPReader->num_channels;
	pdata0 = g_pDPReader->trim_buff;

	BYTE x;

	CString img, str;

	//for (i = 0; i < size; i++) {
	//	x = *(pdata0 + i);
	//	str.Format("%02x ", x);
	//	img += str;	
	//}

	/*for (i = 0; i < packet_size; i++) {
		TxData[7 + i] = packet_data[i];
		eeprom_parity += packet_data[i];
	}

	TxData[7 + packet_size] = eeprom_parity;*/

	p_index = 0;

	for (i = 0; i < npages; i++) {
		// EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata0 + EPKT_SZ * i, EPKT_SZ);
		parity = 0;
		for (j = 0; j < EPKT_SZ; j++) {
			x = *(pdata0 + i * EPKT_SZ + j);
			str.Format("%02x ", x);
			img += str;
			parity += x;
		}
		str.Format("%02x ", parity);
		img += str;

		p_index++;
	}

	//===================

	for (k = 0; k < nchannels; k++) {
		if (g_pTrimReader->Node[k].version != 3) 
			g_pTrimReader->Convert2Int(k);
		
		pdata[k] = g_pTrimReader->Node[k].trim_buff;
		size = g_pTrimReader->WriteTrimBuff(k);
	}

	/*for (k = 0; k < nchannels; k++) {
		for (i = 0; i < size; i++) {
			x = *(pdata[k] + i);
			str.Format("%02x ", x);
			img += str;
		}
	}*/


	for (k = 0; k < nchannels; k++) {		

		for (i = 0; i < NUM_EPKT; i++) {
			// EEPROMWrite(0, npages + NUM_EPKT * nchannels, p_index, pdata[k] + EPKT_SZ * i, EPKT_SZ);
			parity = 0;
			for (j = 0; j < EPKT_SZ; j++) {
				x = *(pdata[k] + i * EPKT_SZ + j);
				str.Format("%02x ", x);
				img += str;
				parity += x;
			}
			str.Format("%02x ", parity);
			img += str;

			p_index++;
		}
	}

	CFileDialog saveDlg(FALSE, _T(".txt"),
		_T("fimg_") + test_report.sys_id, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||"),
		NULL, 0, TRUE);

	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());

		ofs << img;
	}


	

}

// Read flash image

void CTrimDlg::OnBnClickedButtonReadf()
{
	// TODO: Add your control notification handler code here

	CString filter;
	CString strFilePath;
	CString WordBuf[2048];

	filter = "Text File(*.txt)|*.txt|Data File(*.dat)|*.dat||";
	CFileDialog fileDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (fileDlg.DoModal() == IDOK)
	{
		strFilePath = fileDlg.GetPathName();
		SetWindowText(strFilePath);
	}

	CFile InFile;

	CString FileBuf;

	InFile.Open(strFilePath, CFile::modeRead);

	DWORD fl = (DWORD)InFile.GetLength();

	char *buf = new char[fl];

	InFile.Read(buf, fl);

	FileBuf = buf;

	delete buf;

	int ep;

	CString delimit = CString(", \t\r\n");

	int i = 0;

	FileBuf.TrimLeft(delimit);

	while (((ep = FileBuf.FindOneOf(delimit)) != -1) && i < 2000)
	{
		WordBuf[i] = FileBuf.Mid(0, ep);
		int l = FileBuf.GetLength();
		FileBuf = FileBuf.Mid(ep, (l - ep));
		FileBuf.TrimLeft(delimit);
		i++;
	}

	int TotalWords = i;
	FileBuf.Empty();

	InFile.Close();

	//================

	BYTE byte_array[2048];
	
	int y;

	for (i = 0; i < TotalWords; i++) {
		sscanf_s(_T(WordBuf[i]), "%x", &y);
		byte_array[i] = (BYTE)y;
	}

	if (byte_array[0] != 0xa5 && byte_array[1] != 0x01) {
		MessageBox("Wrong format or unsupported version- flash image file");
		return;
	}

	int npages = (int)byte_array[2];

	if (TotalWords < (npages + 16) * (EPKT_SZ + 1)) {
		MessageBox("File corrupted- flash image file");
		return;
	}

	//================

	int index = 0;

	bool parity_ok = true;

	int parity_err = 0;

	BYTE parity;

	CString str, pstr;

	for (index = 0; index < npages + 16; index++) {
		parity = 0;
		for (int i = 0; i < EPKT_SZ + 1; i++) {		// 
			EepromBuff[index][i] = byte_array[index * (EPKT_SZ + 1) + i];

			if (i < EPKT_SZ) {
				parity += byte_array[index * (EPKT_SZ + 1) + i];
			}
			else {
				if (parity != byte_array[index * (EPKT_SZ + 1) + i]) {
					MessageBox(_T("Packet parity error!"));
					parity_ok = false;
				}
			}
		}

		//=======print data============

		if (parity_ok) {
			pstr = " Parity OK.";
		}
		else {
			pstr = " Parity fail";
			parity_err++;
			
		}

		str.Format("flash image read data: %d", index);
		g_pGraDlg->DisplayPollStatus(str);
		for (int i = 0; i < EPKT_SZ; i++) {
			str.Format("%0.2x", byte_array[index * (EPKT_SZ + 1) + i]);
			//			g_pGraDlg->DisplayPollStatus(str);
		}
		str.Format("%0.2x(P)", byte_array[index * (EPKT_SZ + 1) + i]);
		g_pGraDlg->DisplayPollStatus(str + pstr);
	}

	if (parity_err == 0) {
		MessageBox("Flash image read success");
	}
	else {
		MessageBox("Flash image read failure");
	}

	m_DPReader2.CopyEepromBuffAndRestore();			// Compare with with g_DPReader.

	int nchannels = (int)m_DPReader2.num_channels;
	 npages = (int)m_DPReader2.num_pages;
	int sn1 = m_DPReader2.serial_number1;
	int sn2 = m_DPReader2.serial_number2;
	BYTE ml = m_DPReader2.id;

	m_TrimReader2.NumNode = nchannels;

	for (int i = 0; i < nchannels; i++) {
		m_TrimReader2.CopyEepromBuff(i, npages + i * NUM_EPKT);
		m_TrimReader2.RestoreTrimBuff(i);
		m_TrimReader2.Node[i].version = 3;				// So it will use integer version KB matrix and FPN values
	}

	g_pTrimReader = &m_TrimReader2;
	g_pDPReader = &m_DPReader2;

	ResetTrim();

	// dta position file is parsed at GraDlg Init. Need to make sure GraDlg Init is run first.
	if (nchannels < 4) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK4))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime4))->EnableWindow(false);
	}
	if (nchannels < 3) {
		((CButton*)GetDlgItem(IDC_CHECK_TRIM_MASK3))->EnableWindow(false);
		((CEdit*)GetDlgItem(IDC_TRIM_EDIT_IntergrateTime3))->EnableWindow(false);
	}

	CString s;

	if (m_DPReader2.version == 0) {
		s = (char)ml;
		str.Format("%s-%d-%d", s, sn1, sn2);
	}
	else {
		str = m_DPReader2.id_str.c_str();
	}

	// MessageBox(str);
	// SetDlgItemText(1105, str);
	
	test_report.sys_id = str;

	m_tc95 = (float)g_pDPReader->tc95 / 64;
	m_tc55 = (float)g_pDPReader->tc55 / 64;

	CString ostr;
	ostr.Format("%0.3f", m_tc95);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen95, ostr);
	ostr.Format("%0.3f", m_tc55);
	SetDlgItemTextA(IDC_TRIM_EDIT_TempCompen55, ostr);

	test_report.config.Format("%d-%d", g_pDPReader->num_wells, g_pDPReader->num_channels);

	switch (currentLanguage) {
	case language_English:
		g_pGraDlg->DisplayReportStatus("System ID: " + test_report.sys_id + "\r\nTest Time: " + test_report.time + "\r\nNum of Wells and Channels: " + test_report.config);
		break;
	case language_Chinese:
		g_pGraDlg->DisplayReportStatus("System ID (系统编号)： " + test_report.sys_id + "\r\nTest Time (测试时间)： " + test_report.time + "\r\nNum of Wells and Channels(反应池及通道)： " + test_report.config);
		break;
	}

}
