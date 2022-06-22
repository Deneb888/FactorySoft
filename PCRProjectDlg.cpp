
// PCRProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "IntTimeDlg.h"
#include "afxdialogex.h"

#include "SystemConfig.cpp"

using namespace std;

//....................................................................
#include <wtypes.h>
#include <initguid.h>

#define MAX_LOADSTRING 256

extern "C" {

	// This file is in the Windows DDK available from Microsoft.
#include "hidsdi.h"

#include <setupapi.h>
#include <dbt.h>
}
//....................................................................

#ifdef _DEBUG
#define new DEBUG_NEW
//....................................................................
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//....................................................................
#endif
//....................................................................

//....................................................................

//....................................................................
//function prototypes
/*
BOOL DeviceNameMatch(LPARAM lParam);
bool FindTheHID();
LRESULT Main_OnDeviceChange(WPARAM wParam, LPARAM lParam);
void CloseHandles();
void DisplayInputReport();
void DisplayReceivedData(char ReceivedByte);
void GetDeviceCapabilities();
void PrepareForOverlappedTransfer();
void ReadAndWriteToDevice();
void GRAHID_ReadHIDInputReport();
void RegisterForDeviceNotifications();
void GRAHID_WriteHIDOutputReport();

UINT ReadReportThread(LPVOID pParam);	// ¶Á±¨¸æÏß³Ì
*/
//....................................................................

//....................................................................
//Application global variables 
DWORD								ActualBytesRead;
DWORD								BytesRead;
HIDP_CAPS							Capabilities;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
HANDLE								DeviceHandle;
HANDLE								GraphicDeviceHandle;
HANDLE								TemperatureDeviceHandle;
DWORD								dwError;
char								FeatureReport[256];
HANDLE								hDevInfo;
GUID								HidGuid;
ULONG								Length;
LPOVERLAPPED						lpOverLap;
bool								MyDeviceDetected = FALSE; 
CString								MyDevicePathName;
DWORD								NumberOfBytesRead;
DWORD								ReportType;
ULONG								Required;
CString								ValueToDisplay;
OVERLAPPED							GraHID_HIDOverlapped;
OVERLAPPED							TemHID_HIDOverlapped;
HANDLE								GraHID_hEventObject;
HANDLE								TemHID_hEventObject;
CString								GraHidPathName;
CString								TemHidPathName;
char								GraHID_InputReport[HIDREPORTNUM];
char								GraHID_OutputReport[HIDREPORTNUM];
char								TemHID_InputReport[HIDREPORTNUM];
char								TemHID_OutputReport[HIDREPORTNUM];
HANDLE								Gra_ReadHandle;
HANDLE								Gra_WriteHandle;
WORD								Gra_OutReport_Length;
WORD								Gra_InReport_Length;
HANDLE								Tem_ReadHandle;
HANDLE								Tem_WriteHandle;
WORD								Tem_OutReport_Length;
WORD								Tem_InReport_Length;


//These are the vendor and product IDs to look for.
//Uses Lakeview Research's Vendor ID.
//int VendorID = 0x0483;
//int ProductID = 0x5750;

//....................................................................

//*****************************************************
//Global variable definition
//*****************************************************
BYTE TxData[TxNum];		// the buffer of sent data to COMX
BYTE RxData[RxNum];		// the buffer of received data from COMX
CByteArray array;		// the buffer used to send data to COMX

CString RegRecStr;				//
CString Dec_RegRecStr;			//
CString Valid_RegRecStr;		//
CString Valid_Dec_RegRecStr;	//

BYTE rCmd;		//
BYTE rType;		//
BYTE rData;		//
BYTE rEcode;	// Error code. 0: no error=

int mRegFlag;	//

OVERLAPPED ReadOverlapped;			//½ÓÊÕ±¨¸æÓÃµÄOVERLAPPED

CWinThread * pReadReportThread;		//Ö¸Ïò¶Á±¨¸æÏß³ÌµÄÖ¸Õë

DWORD InputLength = 0;

bool GraHidFlag = false;			// graphic HID find flag
bool TemHidFlag = false;			// temperature HID find flag

CString sGraFirmwareVer;			// graphic HID version string
CString sTemFirmwareVer;			// temperature HID version string
CString sUIVer;						// UI version string

BOOL timerCtrFlag = TRUE;			//Í¼Ïñ°åºÍÎÂ¶È°åHID¶ÁÈ¡timer¿ØÖÆ
									//×÷ÓÃ£ºHID¶ÁÈ¡Êý¾ÝÊ±£¬ÔÚ¶Áµ½·µ»ØÊý¾ÝÇ°£¬Í£Ö¹HID·¢ËÍÊý¾Ý
									// HID read timer control for both Gra and Tem HID. Function: stop HID send when reading HID

int	hidReadCnt = 0;					//HID¶ÁÈ¡Êý¾ÝÊÇ£¬timeoutºóÖØÐÂ¶ÁÈ¡¼ÆÊý

//*****************************************************
//External variable definition
//*****************************************************
extern int RegFlag;		// register dialog message flag
extern int GraFlag;		// graphic dialog message flag
extern int TrimFlag;	// trim dialog message flag

extern bool Gra_pageFlag;		// graphic dialog »­Ò³Ñ­»·±êÖ¾
extern bool Gra_videoFlag;		// graphic dialog videoÑ­»·±êÖ¾

extern BYTE RegBuf [regdatanum];	// register dialog transmitted data buffer
extern BYTE GraBuf[GRADATANUM];	// graphic dialog transmitted data buffer


//****************************************************
//Own function prototype 
//****************************************************
unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
int ChangeNum (CString str, int length);									//Ê®Áù½øÖÆ×Ö·û´®×ªÊ®½øÖÆÕûÐÍ
char* EditDataCvtChar (CString strCnv,  char * charRec);	//±à¼­¿òÈ¡Öµ×ª×Ö·û±äÁ¿


 BOOL g_DeviceDetected = false;

 CTrimDlg *g_pTrimDlg = 0;
 CTrimReader *g_pTrimReader = 0;
 CGraDlg *g_pGraDlg;

 CString g_ExpName;

 BOOL g_dirty = false;

 DPReader *g_pDPReader;

 ReportStrings test_report;


//*****************************************************
//Own function
//*****************************************************

//ASIC×Ö·û×ªÊ®Áù½øÖÆº¯Êý
unsigned char AsicConvert (unsigned char i, unsigned char j)
{
	switch (i) 
	{	               
	case 0x30:return (j=0x00);break; 
	case 0x31:return (j=0x01);break; 
	case 0x32:return (j=0x02);break; 
	case 0x33:return (j=0x03);break; 
	case 0x34:return (j=0x04);break; 
	case 0x35:return (j=0x05);break; 
	case 0x36:return (j=0x06);break; 
	case 0x37:return (j=0x07);break; 
	case 0x38:return (j=0x08);break; 
	case 0x39:return (j=0x09);break; 
	case 0x41:
	case 0x61:return (j=0x0A);break;
	case 0x42:	
	case 0x62:return (j=0x0B);break; 
	case 0x43:
	case 0x63:return (j=0x0C);break; 
	case 0x44:
	case 0x64:return (j=0x0d);break;
	case 0x45:	
	case 0x65:return (j=0x0e);break; 
	case 0x46:
	case 0x66:return (j=0x0f);break;
	case 0x20:return (' ');break; 
	default: return(j=0x10);break;
	}
}

//×Ö·û´®×ªÊ®½øÖÆº¯Êý
int ChangeNum (CString str, int length)
{
	char  revstr[16]={0};  
	int   num[16]={0};  
	int   count=1;  
	int   result=0;  
	strcpy_s(revstr,str);  
	for   (int i=length-1;i>=0;i--)  
	{  
		if ((revstr[i]>='0') && (revstr[i]<='9'))  
			num[i]=revstr[i]-48;//×Ö·û0µÄASCIIÖµÎª48
		else if ((revstr[i]>='a') && (revstr[i]<='f'))  
			num[i]=revstr[i]-'a'+10;  
		else if ((revstr[i]>='A') && (revstr[i]<='F'))  
			num[i]=revstr[i]-'A'+10;  
		else  
			num[i]=0;
		result=result+num[i]*count;  
		count=count*16;  
	}  
	return result;
}

//´Ó±à¼­¿òÈ¡Öµ×ªcharÐÍ
 char* EditDataCvtChar (CString strCnv,  char * charRec)
{
	//´ÓRowNum±à¼­¿òÈ¡Öµ
	//	CString RNum;
	char * BSNum;
	//	unsigned char * BANum;
	int CNumByte;
	unsigned char k=0;
	int i,j=0;

	CNumByte=strCnv.GetLength();
	BSNum = new char [CNumByte];
	charRec = new char [CNumByte];

	BSNum = (char*)(LPCSTR)strCnv;

	for(i=0;i<(CNumByte/2);i++)
	{
		charRec[i] = (AsicConvert(BSNum[j],k)<<4) | AsicConvert(BSNum[j+1],k);
		j += 2;
	}

	return charRec;
}

//Ïò´®¿Ú·¢ËÍÊý¾Ý£¬ÆäÖÐ²ÎÊýÎª·¢ËÍÊý¾ÝµÄbyte¸öÊý
void CPCRProjectDlg:: CommSend(int num)
{
/*	int a;

	array.RemoveAll();
	array.SetSize(num);
	for (a=0;a<num;a++)
	{
		array.SetAt(a,TxData[a]);
	}
	m_mscomm.put_Output(COleVariant(array));	//send data
*/
}

//
void CPCRProjectDlg::SendHIDRead()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_ReadHID_event,a,b);
}

//¶Á±¨¸æÏß³Ì
UINT ReadReportThread(LPVOID pParam)
{

	return 0;
}

//....................................................................
LRESULT CPCRProjectDlg::Main_OnDeviceChange(WPARAM wParam, LPARAM lParam)  
{

	//DisplayData("Device change detected.");

#ifdef _DEBUG
//	AfxMessageBox("Device change detected.");
#endif

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

//	FindTheHID();

	switch(wParam) 
	{
		// Find out if a device has been attached or removed.
		// If yes, see if the name matches the device path name of the device we want to access.

	case DBT_DEVICEARRIVAL:		// HID pull in

		if (FindTheHID()) {
			m_GraDlg.DisplayPollStatus("XXXX USB ReattachXXXXX!");

			if (!trim_loaded) {
				//		CString s = m_TrimDlg.ReadFlash();
				//		SetDlgItemText(IDC_STATIC_TRIM, s);

				int result = MessageBox("Read trim data from Flash?", "PCRProject", MB_ICONQUESTION | MB_YESNO);
				if (result == IDYES) {
					CString s = m_TrimDlg.ReadFlash();
					SetDlgItemText(IDC_STATIC_TRIM, s);

					test_report.sys_id = s;
				}
				else {
					MessageBox("Without device trim data, system will not operate properly", "PCRProject", MB_ICONERROR);
				}
			}


			m_TrimDlg.ResetTrim();
			m_OperDlg.ResetParam();
		}

		return TRUE; 

	case DBT_DEVICEREMOVECOMPLETE:		// HID pull out

		FindTheHID();

		m_GraDlg.DisplayPollStatus("====== USB Detached======!");

		return TRUE; 

	default:
		return TRUE; 
	} 

		return TRUE;
}

BOOL CPCRProjectDlg::DeviceNameMatch(LPARAM lParam)
{

	// Compare the device path name of a device recently attached or removed 
	// with the device path name of the device we want to communicate with.

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

//	DisplayData("MyDevicePathName = " + MyDevicePathName);

	if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) 
	{

		PDEV_BROADCAST_DEVICEINTERFACE lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;


		CString DeviceNameString;

		//The dbch_devicetype parameter indicates that the event applies to a device interface.
		//So the structure in LParam is actually a DEV_BROADCAST_INTERFACE structure, 
		//which begins with a DEV_BROADCAST_HDR.

		//The dbcc_name parameter of DevBroadcastDeviceInterface contains the device name. 

		//Compare the name of the newly attached device with the name of the device 
		//the application is accessing (myDevicePathName).

		DeviceNameString = lpdbi->dbcc_name;

		if (DeviceNameString.CompareNoCase(GraHidPathName) == 0)
		{
			GraHidFlag = false;
			SetDlgItemText(IDC_STATIC_GRAHIDSTATUS,"Graphic HID has removed");
			
			return true;
		}
			else
			{
				if (DeviceNameString.CompareNoCase(GraHidPathName) == 0)
				{
					GraHidFlag = false;
					SetDlgItemText(IDC_STATIC_GRAHIDSTATUS,"Temperature HID has removed");

					return true;
				}
				else
					return false;
			}
	}

		
/*
		if ((DeviceNameString.CompareNoCase(MyDevicePathName)) == 0)

		{
			//The name matches.
			return true;
		}
		else
		{
			//It's a different device.
			return false;
		}
*/
	else
	{
		return false;
	}	
}

bool CPCRProjectDlg::FindTheHID()
{
	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
//	DWORD								DeviceUsage;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	bool								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	CString								UsageDescription;

	Length = 0;
	detailData = NULL;
	DeviceHandle=NULL;

	// initial tow HID flags
	GraHidFlag = false;
	TemHidFlag = false;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);	
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	// initial start search status
	MemberIndex = 0;
	LastDevice = FALSE;
	MyDeviceDetected = FALSE;

	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			
			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.			
			If retrieving the structure, set 
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/

			DeviceHandle=CreateFile 
				(detailData->DevicePath, 
				0, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//			HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//			DisplayLastError("CreateFile: ");

			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes 
				(DeviceHandle, 
				&Attributes);	

			switch(Attributes.VendorID)
			{
			case GRA_VID:
				{
					if (Attributes.ProductID == GRA_PID)
					{
						//Both the Vendor ID and Product ID match.

						//Register to receive device notifications.
						RegisterForDeviceNotifications();

						GraHidFlag = true;		// set graphic HID find flag
						GraphicDeviceHandle = DeviceHandle;			// get graphic HID device handle

						MyDeviceDetected = TRUE;
						GraHidPathName = detailData->DevicePath;		// get graphic HID device path name

						//Get the device's capablities.
						GetDeviceCapabilities(GraphicDeviceHandle);
						Gra_OutReport_Length = Capabilities.OutputReportByteLength;
						Gra_InReport_Length = Capabilities.InputReportByteLength;

						// Get a handle for writing Output reports.
						Gra_WriteHandle=CreateFile 
							(detailData->DevicePath, 
							GENERIC_WRITE, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							(LPSECURITY_ATTRIBUTES)NULL,
							OPEN_EXISTING, 
							0, 
							NULL);

						// Get a handle for read Input reports.
						Gra_ReadHandle=CreateFile 
							(detailData->DevicePath, 
							GENERIC_READ, 
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							(LPSECURITY_ATTRIBUTES)NULL, 
							OPEN_EXISTING, 
							FILE_FLAG_OVERLAPPED,
							NULL);

						HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);

					} //if (Attributes.ProductID == Graphic HID ProductID)

					else		//The Product ID doesn't match Graphic HID PID.
					{
						GraHidFlag = false;
					}
					break;
				}
			case TEM_VID:
				{
					if (Attributes.ProductID == TEM_PID)
					{
						//Register to receive device notifications.
						RegisterForDeviceNotifications();

						TemHidFlag = true;		// set temperature HID find flag
						TemperatureDeviceHandle = DeviceHandle;			// get temperature HID device handle

						MyDeviceDetected = TRUE;
						TemHidPathName = detailData->DevicePath;		// get temperature HID device path name

						//Get the device's capablities.
						GetDeviceCapabilities(TemperatureDeviceHandle);
						Tem_OutReport_Length = Capabilities.OutputReportByteLength;
						Tem_InReport_Length = Capabilities.InputReportByteLength;

						// Get a handle for writing Output reports.
						Tem_WriteHandle=CreateFile 
							(detailData->DevicePath, 
							GENERIC_WRITE, 
							FILE_SHARE_READ|FILE_SHARE_WRITE, 
							(LPSECURITY_ATTRIBUTES)NULL,
							OPEN_EXISTING, 
							0, 
							NULL);

						// Get a handle for read Input reports.
						Tem_ReadHandle=CreateFile 
							(detailData->DevicePath, 
							GENERIC_READ, 
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							(LPSECURITY_ATTRIBUTES)NULL, 
							OPEN_EXISTING, 
							FILE_FLAG_OVERLAPPED,
							NULL);

						HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);

					} //if (Attributes.ProductID == Temperature HID ProductID)

					else		//The Product ID doesn't match the temperature HID PID
					{
						TemHidFlag = false;
					}
					break;
				}
			default:
				{					
					break;
				}
			}

		//Free the memory used by the detailData structure (no longer needed).
		free(detailData);

		}  //if (Result != 0)

		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.

			LastDevice=TRUE;

		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.

		MemberIndex = MemberIndex + 1;

	} //do

	while ((LastDevice == FALSE) && ((GraHidFlag == FALSE) || (TemHidFlag == FALSE)));

	if (MyDeviceDetected == FALSE) 
	{
		SetDlgItemText(IDC_STATIC_GRAHIDSTATUS,"Graphic HID Not Detected");
//		SetDlgItemText(IDC_STATIC_TEMHIDSTATUS,"Temperature HID Not Detected");
		
		//added by lixin 20190327
		switch(currentLanguage){
			case language_English:
				SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID Not Detected");
				break;
			case language_Chinese:
				SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID 设备没有接上");
				break;
		} 
		g_DeviceDetected = false;
	}
	else 
	{
		if (GraHidFlag)
			SetDlgItemText(IDC_STATIC_GRAHIDSTATUS,"Graphic HID Detected");
		else
			SetDlgItemText(IDC_STATIC_GRAHIDSTATUS,"Graphic HID Not Detected");

		if (TemHidFlag)
//			SetDlgItemText(IDC_STATIC_TEMHIDSTATUS,"Temperature HID Detected");
			
			//added by lixin 20190327
			switch(currentLanguage){
				case language_English:
					SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID Detected");
					break;
				case language_Chinese:
					SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID 设备接上成功");
					break;
			} 

		else
//			SetDlgItemText(IDC_STATIC_TEMHIDSTATUS,"Temperature HID Not Detected");

			//added by lixin 20190327
			switch(currentLanguage){
				case language_English:
					SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID Not Detected");
					break;
				case language_Chinese: 
					SetDlgItemText(IDC_STATIC_TEMHIDSTATUS, "PDx16 HID 设备没有接上");
					break;
			} 


		g_DeviceDetected = true;
	}

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.

	SetupDiDestroyDeviceInfoList(hDevInfo);
//	DisplayLastError("SetupDiDestroyDeviceInfoList");

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
//	HidD_SetNumInputBuffers(WriteHandle,HIDBUFSIZE);
//	HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	return MyDeviceDetected;
}

void CPCRProjectDlg::CloseHandles()
{
	//Close open handles.

	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
	}

	if (Gra_ReadHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Gra_ReadHandle);
	}

	if (Gra_WriteHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Gra_WriteHandle);
	}
}

void CPCRProjectDlg::DisplayInputReport()
{
	USHORT	ByteNumber;
	CHAR	ReceivedByte;

	//Display the received data in the log and the Bytes Received List boxes.
	//Start at the top of the List Box.

	m_BytesReceived.ResetContent();

	//Step through the received bytes and display each.

	for (ByteNumber=0; ByteNumber < Capabilities.InputReportByteLength; ByteNumber++)
	{
		//Get a byte.

		ReceivedByte = GraHID_InputReport[ByteNumber];

		//Display it.

		DisplayReceivedData(ReceivedByte);
	}
}

void CPCRProjectDlg::DisplayReceivedData(char ReceivedByte)
{
	//Display data received from the device.

	CString	strByteRead;

	//Convert the value to a 2-character Cstring.

	strByteRead.Format("%02X", ReceivedByte);
	strByteRead = strByteRead.Right(2); 

	//Display the value in the Bytes Received List Box.

	m_BytesReceived.InsertString(-1, strByteRead);

	//Display the value in the log List Box (optional).
	//MessageToDisplay.Format("%s%s", "Byte 0: ", strByteRead); 
	//DisplayData(MessageToDisplay);	
	//UpdateData(false);
}

void CPCRProjectDlg::GetDeviceCapabilities(HANDLE pHidHandle)
{
	//Get the Capabilities structure for the device.

	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData 
		(pHidHandle, 
		&PreparsedData);
//	DisplayLastError("HidD_GetPreparsedData: ");

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/
	
	HidP_GetCaps 
		(PreparsedData, 
		&Capabilities);
//	DisplayLastError("HidP_GetCaps: ");

	//No need for PreparsedData any more, so free the memory it's using.

	HidD_FreePreparsedData(PreparsedData);
//	DisplayLastError("HidD_FreePreparsedData: ") ;
}

void CPCRProjectDlg::PrepareForOverlappedTransfer()
{
	//Get a handle to the device for the overlapped ReadFiles.

	Gra_ReadHandle=CreateFile 
		(detailData->DevicePath, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED,
		NULL);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//		HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//	DisplayLastError("CreateFile (ReadHandle): ");

	//Get an event object for the overlapped structure.

	/*API function: CreateEvent
	Requires:
	  Security attributes or Null
	  Manual reset (true). Use ResetEvent to set the event object's state to non-signaled.
	  Initial state (true = signaled) 
	  Event object name (optional)
	Returns: a handle to the event object
	*/

	if (GraHID_hEventObject == 0)
	{
		GraHID_hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
//	DisplayLastError("CreateEvent: ") ;

	//Set the members of the overlapped structure.

	GraHID_HIDOverlapped.hEvent = GraHID_hEventObject;
	GraHID_HIDOverlapped.Offset = 0;
	GraHID_HIDOverlapped.OffsetHigh = 0;
	}
}

void CPCRProjectDlg::ReadAndWriteToDevice()
{
	//If necessary, find the device and learn its capabilities.
	//Then send a report and request a report.

	//Clear the List Box (optional).
	//m_ResultsList.ResetContent();

//	DisplayData("***HID Test Report***");
//	DisplayCurrentTime();

	//If the device hasn't been detected already, look for it.

	if (MyDeviceDetected==FALSE)
	{
		MyDeviceDetected=FindTheHID();
	}

	// Do nothing if the device isn't detected.

	if (MyDeviceDetected==TRUE)
	{
		//if (FindTheHID()) {
			m_TrimDlg.ResetTrim();
			m_OperDlg.ResetParam();
		//}

		//Write a report to the device.

		GRAHID_WriteHIDOutputReport();

		//Active read message

		SendHIDRead();
	} 
}


// not used

void CPCRProjectDlg::GRAHID_WriteHIDOutputReport()
{
	ASSERT(0);
	//Send a report to the device.

	DWORD	BytesWritten = 0;
	INT		Index =0;
	ULONG	Result;
	CString	strBytesWritten = "";

	UpdateData(true);

	//The first byte is the report number.

	GraHID_OutputReport[0]=0;

	for (int i=1; i<TxNum+1; i++)
	 GraHID_OutputReport[i] = TxData[i-1];

	/*
		API Function: WriteFile
		Sends a report to the device.
		Returns: success or failure.
		Requires:
		A device handle returned by CreateFile.
		A buffer that holds the report.
		The Output Report length returned by HidP_GetCaps,
		A variable to hold the number of bytes written.
	*/

		if (Gra_WriteHandle != INVALID_HANDLE_VALUE)
			{
			Result = WriteFile 
			(Gra_WriteHandle, 
			GraHID_OutputReport, 
//			Capabilities.OutputReportByteLength,
			Gra_OutReport_Length,
			&BytesWritten, 
			NULL);
		}

		//Display the result of the API call and the report bytes.

		if (!Result)
			{
			//The WriteFile failed, so close the handles, display a message,
			//and set MyDeviceDetected to FALSE so the next attempt will look for the device.

			CloseHandles();
			SetDlgItemText(IDC_STATICOpenComm,"Can't write to graphic device");
			MyDeviceDetected = FALSE;
			g_DeviceDetected = FALSE;
			}

}

// not used

void CPCRProjectDlg::GRAHID_ReadHIDInputReport()
{
	ASSERT(0);

	// Retrieve an Input report from the device.

	DWORD	Result;
	
	//The first byte is the report number.
	GraHID_InputReport[0]=0;

//	DisplayLastError("ReadFile: ") ;

	/*API call:WaitForSingleObject
	'Used with overlapped ReadFile.
	'Returns when ReadFile has received the requested amount of data or on timeout.
	'Requires an event object created with CreateEvent
	'and a timeout value in milliseconds.
	*/

	if (GraHID_hEventObject == 0)
	{
		GraHID_hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
		//Set the members of the overlapped structure.

		GraHID_HIDOverlapped.hEvent = GraHID_hEventObject;
		GraHID_HIDOverlapped.Offset = 0;
		GraHID_HIDOverlapped.OffsetHigh = 0;
	}


/*API call:ReadFile
	'Returns: the report in GraHID_InputReport.
	'Requires: a device handle returned by CreateFile
	'(for overlapped I/O, CreateFile must be called with FILE_FLAG_OVERLAPPED),
	'the Input report length in bytes returned by HidP_GetCaps,
	'and an overlapped structure whose hEvent member is set to an event object.
	*/

	if (Gra_ReadHandle != INVALID_HANDLE_VALUE)
		{
		Result = ReadFile 
		(Gra_ReadHandle, 
		GraHID_InputReport, 
//		Capabilities.InputReportByteLength,
		Gra_InReport_Length,
		&NumberOfBytesRead,
		(LPOVERLAPPED) &GraHID_HIDOverlapped
		); 
		}

	Result = WaitForSingleObject 
		(GraHID_hEventObject, 
		HIDREADTIMEOUT);


	CString strtest;		//½ÓÊÕµ½ÏÂÎ»»ú·µ»ØµÄ´íÎóµÄÃüÁîbuffer
	strtest.Empty();
	CString strtemp;		//ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®Áù½øÖÆ
	strtemp.Empty();
	CString DStrtemp;		//ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®½øÖÆ
	DStrtemp.Empty();
	CString Valid_strtemp;	//ÓÐÐ§ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®Áù½øÖÆ
	Valid_strtemp.Empty();
	CString Valid_DStrtemp;	//ÓÐÐ§ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®½øÖÆ
	Valid_DStrtemp.Empty();
	long k = 0;

	InputLength = Capabilities.InputReportByteLength;
	if (InputLength>65)
		AfxMessageBox("The length of received data is over 65");

	switch (Result)
	{
	case WAIT_OBJECT_0:
		{
				//½«¸÷×ª»»×Ö·û´®bufferÇåÁã
				RegRecStr.Empty();	// ClearÖ®Ç°µÄÏÔÊ¾Êý¾Ý
				Dec_RegRecStr.Empty();	// ClearÖ®Ç°µÄÏÔÊ¾Êý¾Ý
				Valid_RegRecStr.Empty();
				Valid_Dec_RegRecStr.Empty();

				//´¦Àí½ÓÊÕµÄÊý¾Ý
				for (k=0;k<HIDREPORTNUM-1;k++)
					RxData[k] = GraHID_InputReport[k+1];	//½«½ÓÊÕµÄÊý¾Ý°´×Ö½Ú·ÖÅäµ½×Ö½Ú´æ´¢buffer(RxData[200])
				//È¡³ö·µ»ØµÄÃüÁîºÍtype
				rCmd = RxData[2];	//È¡³öÏÂÎ»»ú·µ»ØµÄÃüÁî
				rType = RxData[4];	//È¡³öÏÂÎ»»ú·µ»ØµÄtype
				strtest.Format("%2x",rCmd);

				//½«·µ»ØµÄÊý¾ÝÕûÀí³É¸÷ËùÐèµÄ×Ö·û´®
				//È¡ËùÓÐÊý¾Ý
				for(k=0;k<HIDREPORTNUM;k++)			 //½«×Ö·ûÊý×éµÄ¸÷×Ö·û×ª³É×Ö·û´®
				{
					strtemp.Format("%02X ",RxData[k]);			//Ã¿¸ö×Ö·û×ª»¯£¬Ê®Áù½øÖÆÏÔÊ¾,ÈçÓÐ¡°0¡±Ò²»áÏÔÊ¾Èç¡°02¡±
					//Èô²»ÒªÏÔÊ¾¡°0¡±£¬strtemp.Format("%x",bt)
					DStrtemp.Format("%02d",RxData[k]);			//Ê®½øÖÆÏÔÊ¾£¬ÆäËüÈçÉÏ

					RegRecStr+=strtemp;					//½«Ã¿¸öÊ®Áù½øÖÆ×Ö·û×ª³ÉµÄ×Ö·û´®×éºÏ³ÉÒ»¸ö×Ö·û´®
					//Ê®Áù½øÖÆ×ª»¯ÖÐÏàÁÚ¸÷byte¼ä×Ô´ø¿Õ¸ñ
					//½ÓÊÕµÄÃ¿¸ö°üµÄÊý¾Ý¶¼´æÔÚ¸Ã×Ö·û´®£¬³ý·Ç¶Ô¸Ã×Ö·û´®ÇåÁã£¬·ñÔòÖ®Ç°ËùÓÐÊý¾Ý±£´æ

					Dec_RegRecStr += (DStrtemp+" ");	//½«Ã¿¸öÊ®½øÖÆ×Ö·û×ª³ÉµÄ×Ö·û´®×éºÏ³ÉÒ»¸ö×Ö·û´®
					//Ê®½øÖÆ×ª»¯ÏàÁÚÁ½¸ö×Ö·û²»×Ô´ø¿Õ¸ñ£¬ÔÚÕâÀïÌí¼Ó
					//½ÓÊÕµÄÃ¿¸ö°üµÄÊý¾Ý¶¼´æÔÚ¸Ã×Ö·û´®£¬³ý·Ç¶Ô¸Ã×Ö·û´®ÇåÁã£¬·ñÔòÖ®Ç°ËùÓÐÊý¾Ý±£´æ
				}
				//Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
				//		RegRecStr += "\r\n";
				//		Dec_RegRecStr += "\r\n";

				//¸ù¾ÝÏÂÎ»»ú·µ»ØµÄÃüÁî£¬½øÈë¸÷×ÔµÄÏûÏ¢´¦Àí³ÌÐò
				switch(rCmd)
				{
				case GRACMD:
					{
						if (RxData[5] == 0xF1)
						{
							MessageBox("Error: Chip sample abnormally");
							PCRType = 0;
							memset(&RxData,0,sizeof(RxData));
							return;
						}
						else
						{
							PCRTypeFilterClass = rType & 0x0F;		// ÅÐ¶Ï·µ»ØµÄÀà±ð£¬ÊÇ»­ÐÐ¡¢Ò³»¹ÊÇvedio
							PCRTypeFilterNum = rType & 0xF0;		// ÅÐ¶ÏÊÇµÚ¼¸¸öPCR·µ»ØµÄÊý¾Ý
							//È¡ÏàÓ¦µÄÓÐÐ§Êý¾Ý
							if ((PCRTypeFilterClass==0x01)|(PCRTypeFilterClass==0x02)|(PCRTypeFilterClass==0x03) )		//·µ»Ø12 pixel Êý¾ÝÊ±
							{	
								// Çø·ÖÐòºÅ
								switch(PCRTypeFilterNum)
								{
								case 0x00:
									PCRNum = 1;
									break;
								case 0x10:
									PCRNum = 2;
									break;
								case 0x20:
									PCRNum = 3;
									break;
								case 0x30:
									PCRNum = 4;
									break;
								default:
									break;
								}

								if (RxData[5]==0x0b)		// HID¶Áµ½µÚ12ÐÐºóÍ£Ö¹¶ÁÈ¡
									Gra_pageFlag = false;
								else
									Gra_pageFlag = true;

								for(k=0; k<26; k++)
								{
									Valid_strtemp.Format("%02X",RxData[k+6]);	//Ê®Áù½øÖÆÏÔÊ¾×Ö·û´®buffer, Ã¿¸öbyte¼ä¼Ó¿Õ¸ñ
									Valid_DStrtemp.Format("%02d",RxData[k+6]);	//Ê®½øÖÆÏÔÊ¾×Ö·û´®buffer, Ã¿¸öbyte¼ä¼Ó¿Õ¸ñ

									Valid_RegRecStr += (Valid_strtemp+" ");
									Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
								}
								//		Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
								//		Valid_RegRecStr += "\r\n";
								//		Valid_Dec_RegRecStr += "\r\n";
							}
							else
							{
								if ((PCRTypeFilterClass==0x07)|(PCRTypeFilterClass==0x08)|(PCRTypeFilterClass==0x0b))	//·µ»Ø24 pixel Êý¾ÝÊ±
								{
									// Çø·ÖÐòºÅ
									switch(PCRTypeFilterNum)
									{
									case 0x00:
										PCRNum = 1;
										break;
									case 0x10:
										PCRNum = 2;
										break;
									case 0x20:
										PCRNum = 3;
										break;
									case 0x30:
										PCRNum = 4;
										break;
									default:
										break;
									}

									if (RxData[5]==0x17)	// HID¶Áµ½µÚ24ÐÐºóÍ£Ö¹¶ÁÈ¡
										Gra_pageFlag = false;
									else
										Gra_pageFlag = true;

									for(k=0; k<50; k++)
									{
										Valid_strtemp.Format("%02X",RxData[k+6]);	//Ê®Áù½øÖÆÏÔÊ¾×Ö·û´® buffer, Ã¿¸ö byte ¼ä¼Ó¿Õ¸ñ
										Valid_DStrtemp.Format("%02d",RxData[k+6]);	//Ê®½øÖÆÏÔÊ¾×Ö·û´® buffer, Ã¿¸ö byte ¼ä¼Ó¿Õ¸ñ

										Valid_RegRecStr += (Valid_strtemp+" ");
										Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
									}
									//Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
									//						Valid_RegRecStr += "\r\n";
									//						Valid_Dec_RegRecStr += "\r\n";
								}
								else	//ÆäËütype´ý¶¨
								{
									Valid_RegRecStr.Empty();
									Valid_Dec_RegRecStr.Empty();
								}
							}

							//Æô¶¯Graphic dialogÏûÏ¢´¦Àí³ÌÐò
							m_GraDlg.SendMessage(UM_GRAPROCESS);
						}				
						break;
					}
				case POLLINGRACMD:
					{
						if (rType == 0x01)
						{
							BYTE rData = 0;
							rData = RxData[5] & 0x0F;
							if (rData)		// pollingµ½ÓÐÐ§·µ»ØÊý¾Ý
							{
								PollingGraTimerFlag = FALSE;	// ÔÝÍ£graphic HID polling

								CString str;
								str.Format("Stop polling Gra HID...(rData: %x)", rData);
								m_GraDlg.DisplayPollStatus(str);

								// ÏÔÊ¾graphic dialog 
								// m_GraDlg.ShowWindow(SW_SHOW);	
								CRect tabRect;    // ±êÇ©¿Ø¼þ¿Í»§ÇøµÄRect     
								// »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect£¬²¢¶ÔÆäµ÷Õû£¬ÒÔÊÊºÏ·ÅÖÃ±êÇ©Ò³     
								m_tab.GetClientRect(&tabRect);     
								tabRect.left += 1;     
								tabRect.right -= 1;     
								tabRect.top += 25;     
								tabRect.bottom -= 1; 

								m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
								m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								
								// ·¢ËÍÒ³Í¼ÏñÏÔÊ¾ÃüÁî
								m_GraDlg.CaptureFrame(true);	
							}
						}
						break;
					}
				case READCMD:
					{
						if (rType == 0x27)
						{
							int iGraFirmwareVer;
							int iYear,iMonth,iDay;
							iGraFirmwareVer = (RxData[6]<<8) | RxData[7];
							iYear = RxData[8];
							iMonth = RxData[9];
							iDay =RxData[10];

							sGraFirmwareVer.Empty();

							switch(RxData[5])
							{
							case 0x01:
								{
									sGraFirmwareVer.Format("GraHID port is temperature HID. Version is: %.4d    %.4d - %.2d - %.2d",iGraFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x02:
								{
									sGraFirmwareVer.Format("GraHID port is 4 channels PCR HID. Version is: %.4d    %.4d - %.2d - %.2d",iGraFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x03:
								{
									sGraFirmwareVer.Format("GraHID port is 1 channel PCR BULK. Version is: %.4d    %.4d - %.2d - %.2d",iGraFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x04:
								{
									sGraFirmwareVer.Format("GraHID port is 1 channel PCR HID. Version is: %.4d    %.4d - %.2d - %.2d",iGraFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							default:
								sGraFirmwareVer.Format("Return version data is wrong");
								break;;
							}
						}
						break;
					}
				default:
					{
						//				AfxMessageBox(strtest);
						break;
					}
				}
					
		break;
		}
	case WAIT_TIMEOUT:
		{
		SetDlgItemText(IDC_STATICOpenComm, "GraHID ReadFile timeout");

		/*API call: CancelIo
		Cancels the ReadFile
        Requires the device handle.
        Returns non-zero on success.
		*/
		
		Result = CancelIo(Gra_ReadHandle);
		
		//A timeout may mean that the device has been removed. 
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.
		CloseHandles();
		MyDeviceDetected = FALSE;
		g_DeviceDetected = FALSE;

		break;
		}
	default:
		{
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.

		CloseHandles();
		SetDlgItemText(IDC_STATICOpenComm,"GraHID Can't read from device");
		MyDeviceDetected = FALSE;
		g_DeviceDetected = FALSE;
		break;
		}
	}

	/*
	API call: ResetEvent
	Sets the event object to non-signaled.
	Requires a handle to the event object.
	Returns non-zero on success.
	*/

	ResetEvent(GraHID_hEventObject);

	//Display the report data.
	DisplayInputReport();

}


void CPCRProjectDlg::TEMHID_WriteHIDOutputReport()
{
	//Send a report to the device.
	if (!g_DeviceDetected) return;				// Zhimin added. Skip if lost connection, for polling stuff, not an issue.

	DWORD	BytesWritten = 0;
	INT		Index =0;
	ULONG	Result;
	CString	strBytesWritten = "";

	ASSERT(timerCtrFlag);
	//Í¬Ê±´øÍ¼Ïñ°åºÍÎÂ¶È°å±ÜÃâ´Ó»ú·µ»ØÊý¾ÝÑÓ³ÙÆÚ¼ätimer·¢ËÍÖ¸Áî
	//µÈ´ý´Ó»ú·µ»Ø
	while (!timerCtrFlag);		// if read hid in progress, dont write


	UpdateData(true);

	//The first byte is the report number.

	TemHID_OutputReport[0]=0;

	for (int i=1; i<TxNum+1; i++)
	 TemHID_OutputReport[i] = TxData[i-1];

	/*
		API Function: WriteFile
		Sends a report to the device.
		Returns: success or failure.
		Requires:
		A device handle returned by CreateFile.
		A buffer that holds the report.
		The Output Report length returned by HidP_GetCaps,
		A variable to hold the number of bytes written.
	*/

		if (Tem_WriteHandle != INVALID_HANDLE_VALUE)
			{
			Result = WriteFile 
			(Tem_WriteHandle, 
			TemHID_OutputReport, 
//			Capabilities.OutputReportByteLength,
			Tem_OutReport_Length,
			&BytesWritten, 
			NULL);
		}

		//Display the result of the API call and the report bytes.

		if (!Result)
			{
			//The WriteFile failed, so close the handles, display a message,
			//and set MyDeviceDetected to FALSE so the next attempt will look for the device.

			CloseHandles();
			SetDlgItemText(IDC_STATICOpenComm,"Can't write to temperature HID device");
			MyDeviceDetected = FALSE;
			g_DeviceDetected = FALSE;
			}

		//Í¬Ê±´øÍ¼Ïñ°åºÍÎÂ¶È°å±ÜÃâ´Ó»ú·µ»ØÊý¾ÝÑÓ³ÙÆÚ¼ätimer·¢ËÍÖ¸Áî
		timerCtrFlag = FALSE;		// stop timers from sending more write request until read finishes.
}

int my_rData = 0;

void CPCRProjectDlg::TEMHID_ReadHIDInputReport()
{

	if (!g_DeviceDetected) return;					// Zhimin added. Skip if lost connection

	// Retrieve an Input report from the device.

	DWORD	Result;
	
	//The first byte is the report number.
	TemHID_InputReport[0]=0;

//	DisplayLastError("ReadFile: ") ;

	/*API call:WaitForSingleObject
	'Used with overlapped ReadFile.
	'Returns when ReadFile has received the requested amount of data or on timeout.
	'Requires an event object created with CreateEvent
	'and a timeout value in milliseconds.
	*/

	if (TemHID_hEventObject == 0)
	{
		TemHID_hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
		//Set the members of the overlapped structure.

		TemHID_HIDOverlapped.hEvent = TemHID_hEventObject;
		TemHID_HIDOverlapped.Offset = 0;
		TemHID_HIDOverlapped.OffsetHigh = 0;
	}

	/*API call:ReadFile
	'Returns: the report in GraHID_InputReport.
	'Requires: a device handle returned by CreateFile
	'(for overlapped I/O, CreateFile must be called with FILE_FLAG_OVERLAPPED),
	'the Input report length in bytes returned by HidP_GetCaps,
	'and an overlapped structure whose hEvent member is set to an event object.
	*/

	if (Tem_ReadHandle != INVALID_HANDLE_VALUE)
		{
		Result = ReadFile 
		(Tem_ReadHandle, 
		TemHID_InputReport, 
//		Capabilities.InputReportByteLength,
		Tem_InReport_Length,
		&NumberOfBytesRead,
		(LPOVERLAPPED) &TemHID_HIDOverlapped
		); 
		}


	Result = WaitForSingleObject 
		(TemHID_hEventObject, 
		HIDREADTIMEOUT);


	CString strtest;		//½ÓÊÕµ½ÏÂÎ»»ú·µ»ØµÄ´íÎóµÄÃüÁîbuffer
	strtest.Empty();
	CString strtemp;		//ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®Áù½øÖÆ
	strtemp.Empty();
	CString DStrtemp;		//ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®½øÖÆ
	DStrtemp.Empty();
	CString Valid_strtemp;	//ÓÐÐ§ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®Áù½øÖÆ
	Valid_strtemp.Empty();
	CString Valid_DStrtemp;	//ÓÐÐ§ÁÙÊ±½ÓÊÕ×Ö·û´®buffer,ÔÝ´æÒ»¸öbyteÊý¾Ý£¬Ê®½øÖÆ
	Valid_DStrtemp.Empty();
	long k = 0;

	InputLength = Capabilities.InputReportByteLength;
	if (InputLength>65)
		AfxMessageBox("The length of received data is over 65");

	switch (Result)
	{
	case WAIT_OBJECT_0:
		{
			//Í¬Ê±´øÍ¼Ïñ°åºÍÎÂ¶È°å±ÜÃâ´Ó»ú·µ»ØÊý¾ÝÑÓ³ÙÆÚ¼ätimer·¢ËÍÖ¸Áî
			//´Ó»úÒÑ·µ»ØÊý¾Ý
			timerCtrFlag = TRUE;	//ÊÕµ½ÓÐÐ§Êý¾Ý£¬ÔÊÐíÍ¼Ïñ°åºÍÎÂ¶È°åtimer·¢ËÍHIDÖ¸Áî. Upon successful read, allow timers to send more write requests.
			hidReadCnt = 0;			//ÖØ¸´·¢ËÍ¶ÁÈ¡¼ÆÊýÇåÁã

				//½«¸÷×ª»»×Ö·û´®bufferÇåÁã
				RegRecStr.Empty();	// ClearÖ®Ç°µÄÏÔÊ¾Êý¾Ý
				Dec_RegRecStr.Empty();	// ClearÖ®Ç°µÄÏÔÊ¾Êý¾Ý
				Valid_RegRecStr.Empty();
				Valid_Dec_RegRecStr.Empty();

				//´¦Àí½ÓÊÕµÄÊý¾Ý
				for (k=0;k<HIDREPORTNUM-1;k++)
					RxData[k] = TemHID_InputReport[k+1];	//½«½ÓÊÕµÄÊý¾Ý°´×Ö½Ú·ÖÅäµ½×Ö½Ú´æ´¢buffer(RxData[200])
				//È¡³ö·µ»ØµÄÃüÁîºÍtype
				rCmd = RxData[2];	//È¡³öÏÂÎ»»ú·µ»ØµÄÃüÁî
				rType = RxData[4];	//È¡³öÏÂÎ»»ú·µ»ØµÄtype
				rData = RxData[5];	//È¡³öÏÂÎ»»ú·µ»ØµÄµÚÒ»¸öÓÐÐ§data
				rEcode = RxData[1];		// Err code

				if (rEcode != 0 || rCmd == 0x13) {
					strtest.Format("Rcv-Cmd: %2x, Type: %2x, ErrCode: %2x",rCmd, rType, rEcode);
					m_GraDlg.DisplayPollStatus(strtest);

					TRACE("Rcv-Cmd: %2x, Type: %2x, ErrCode: %2x", rCmd, rType, rEcode);
				}

				//½«·µ»ØµÄÊý¾ÝÕûÀí³É¸÷ËùÐèµÄ×Ö·û´®
				//È¡ËùÓÐÊý¾Ý
				for(k=0;k<HIDREPORTNUM;k++)			 //½«×Ö·ûÊý×éµÄ¸÷×Ö·û×ª³É×Ö·û´®
				{
					strtemp.Format("%02X ",RxData[k]);			//Ã¿¸ö×Ö·û×ª»¯£¬Ê®Áù½øÖÆÏÔÊ¾,ÈçÓÐ¡°0¡±Ò²»áÏÔÊ¾Èç¡°02¡±
					//Èô²»ÒªÏÔÊ¾¡°0¡±£¬strtemp.Format("%x",bt)
					DStrtemp.Format("%02d",RxData[k]);			//Ê®½øÖÆÏÔÊ¾£¬ÆäËüÈçÉÏ

					RegRecStr+=strtemp;					//½«Ã¿¸öÊ®Áù½øÖÆ×Ö·û×ª³ÉµÄ×Ö·û´®×éºÏ³ÉÒ»¸ö×Ö·û´®
					//Ê®Áù½øÖÆ×ª»¯ÖÐÏàÁÚ¸÷byte¼ä×Ô´ø¿Õ¸ñ
					//½ÓÊÕµÄÃ¿¸ö°üµÄÊý¾Ý¶¼´æÔÚ¸Ã×Ö·û´®£¬³ý·Ç¶Ô¸Ã×Ö·û´®ÇåÁã£¬·ñÔòÖ®Ç°ËùÓÐÊý¾Ý±£´æ

					Dec_RegRecStr += (DStrtemp+" ");	//½«Ã¿¸öÊ®½øÖÆ×Ö·û×ª³ÉµÄ×Ö·û´®×éºÏ³ÉÒ»¸ö×Ö·û´®
					//Ê®½øÖÆ×ª»¯ÏàÁÚÁ½¸ö×Ö·û²»×Ô´ø¿Õ¸ñ£¬ÔÚÕâÀïÌí¼Ó
					//½ÓÊÕµÄÃ¿¸ö°üµÄÊý¾Ý¶¼´æÔÚ¸Ã×Ö·û´®£¬³ý·Ç¶Ô¸Ã×Ö·û´®ÇåÁã£¬·ñÔòÖ®Ç°ËùÓÐÊý¾Ý±£´æ
				}
				//Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
				//		RegRecStr += "\r\n";
				//		Dec_RegRecStr += "\r\n";

				//¸ù¾ÝÏÂÎ»»ú·µ»ØµÄÃüÁî£¬½øÈë¸÷×ÔµÄÏûÏ¢´¦Àí³ÌÐò
				switch(rCmd)
				{
				case READCMD:
					{
						if ((rType == 0x21) | (rType == 0x22))
							m_OperDlg.SendMessage(UM_OPERLEDPROCESS);
						if ((rType == 0x11) | (rType == 0x12) | (rType == 0x2d))
							m_TrimDlg.SendMessage(UM_TRIMPROCESS);
						if (rType == 0x27)
						{
							int plen = RxData[3];	// packet length
							int iTemFirmwareVer;
							int iTemFirmwareVer1, iTemFirmwareVer2;
							int iYear,iMonth,iDay;
							iTemFirmwareVer = (RxData[6]<<8) | RxData[7];
							iTemFirmwareVer1 = RxData[6];
							iTemFirmwareVer2 = RxData[7];
							iYear = RxData[8];
							iMonth = RxData[9];
							iDay =RxData[10];

							CString vstr;

							sTemFirmwareVer.Empty();

							switch(RxData[5])
							{
							case 0x01:
								{
									sTemFirmwareVer.Format("TemHID port is temperature HID. Version is: %.4d    %.4d - %.2d - %.2d",iTemFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x02:
								{
									sTemFirmwareVer.Format("TemHID port is 4 channels PCR HID. Version is: %.4d    %.4d - %.2d - %.2d",iTemFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x03:
								{
									sTemFirmwareVer.Format("TemHID port is 1 channel PCR BULK. Version is: %.4d    %.4d - %.2d - %.2d",iTemFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x04:
								{
									sTemFirmwareVer.Format("TemHID port is 1 channel PCR HID. Version is: %.4d    %.4d - %.2d - %.2d",iTemFirmwareVer,iYear,iMonth,iDay);
									break;
								}
							case 0x10:
								{
									sTemFirmwareVer.Format(/*TemHID port is PCR HID. */"Main Board Version: %.2d.%.2d (%.4d-%.2d-%.2d)", iTemFirmwareVer1, iTemFirmwareVer2, 2000+iYear, iMonth, iDay);
									break;
								}
							default:
								sTemFirmwareVer.Format("Return version data is wrong");
								break;
							}

							if (plen > 7) {			// There appears to be an error in data ordering in Rico's documentation
								iTemFirmwareVer1 = RxData[12];
								iTemFirmwareVer2 = RxData[13];
								iYear = RxData[14];
								iMonth = RxData[15];
								iDay = RxData[16];

								vstr.Format(";\r\nTemp Control Board Version : %.2d.%.2d (%.4d - %.2d - %.2d)", iTemFirmwareVer1, iTemFirmwareVer2, 2000+iYear, iMonth, iDay);
								sTemFirmwareVer += vstr;

								iTemFirmwareVer1 = RxData[18];
								iTemFirmwareVer2 = RxData[19];
								iYear = RxData[20];
								iMonth = RxData[21];
								iDay = RxData[22];

								vstr.Format(";\r\nImage Board Version : %.2d.%.2d (%.4d - %.2d - %.2d)", iTemFirmwareVer1, iTemFirmwareVer2, 2000 + iYear, iMonth, iDay);
								sTemFirmwareVer += vstr;
							}
						}
						break;
					}
				case TEMPCMD:
					{
						if (/*(rType == 0x02) | */(rType == 0x0a))
							m_OperDlg.SendMessage(UM_OPERPROCESS);
						if (rType == 0x05)
							m_OperDlg.SendMessage(UM_OPERLEDPROCESS);
						
					if (rType == 0x02) {
						m_GraDlg.SendMessage(UM_GRACYCPOLLPROC);
					}
					break;
					}
				case CYCCMD:
					{
						if ((rType == 0x01) | (rType == 0x02))
							m_OperDlg.SendMessage(UM_OPERPROCESS);
						if ((rType == 0x05) | (rType == 0x06))		// overshoot read data
							m_OperDlg.SendMessage(UM_OPEROVERSHOOTPROCESS);
						if (rType == 0x0d)
							m_GraDlg.SendMessage(UM_GRACYCPOLLPROC);
						if (rType == 0x15)		// ÊÇ·ñ¼ÌÐøpolling grahpic HIDÈ·ÈÏ
						{
							BYTE rData = 0;
							rData = RxData[5] & 0x0F;		// £¿£¿£¿Ê²Ã´ÓÃ£¨Í£Ö¹polling£¿£©

							my_rData = (int)rData;

							m_GraDlg.DisplayCyclerState(my_rData);

							if ((rData == 0) | (ReceiveTemCycNum == TemperCycNum))		// Í£Ö¹polling graphic
							{
								// Zhimin added Debug code
//								CString s;

//								s.Format("Cycler stop: flag: %d, cycNum: %d", rData, ReceiveTemCycNum);
//								MessageBox(s);

								//=========================

								// reset flag
								ReceiveTemCycNum = 0;
								PollingGraTimerFlag = FALSE;
								PollingGraMode = FALSE;
								m_GraDlg.KillTimer(2);

								m_GraDlg.DisplayPollStatus("Stop polling Gra HID (kill timer 2)");

								m_GraDlg.CoolingDown();

								AutoSampleFlag = FALSE;

								// ÏÔÊ¾operation dialog
								//									m_OperDlg.ShowWindow(SW_SHOW);
/*								CRect tabRect;    // ±êÇ©¿Ø¼þ¿Í»§ÇøµÄRect
								// »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect£¬²¢¶ÔÆäµ÷Õû£¬ÒÔÊÊºÏ·ÅÖÃ±êÇ©Ò³
								m_tab.GetClientRect(&tabRect);
								tabRect.left += 1;
								tabRect.right -= 1;
								tabRect.top += 25;
								tabRect.bottom -= 1;

								m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
*/
							}
							else {    // maybe if rData == 2
								PollingGraTimerFlag = TRUE;

								CString str;
								str.Format("Polling Gra HID...(cycle #: %d)", ReceiveTemCycNum);
								m_GraDlg.DisplayPollStatus(str);
							}
						}
						break;
					}
				case PIDREADCFGCMD:
					{
						if ((rType == 0x01))
						m_OperDlg.SendMessage(UM_OPERPIDPROCESS);
						break;
					}
				// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				/*
				case TRIGERCMD:
					{
						if (rType == 0x24)
						{
							m_GraDlg.SendMessage(UM_GRAPOLLINGPROCESS);
						}
						break;
					}
				*/
				case PIDREADCYCCMD:
					{
//						if (rType != 0x04 && rType != 0x09 && AutoSampleFlag)
//						if ((rType == 0x04 || rType == 0x0b) && AutoSampleFlag)
						if ((rType == 0x09 || rType == 0x03 || rType == 0x0b) && AutoSampleFlag)
							m_GraDlg.SendMessage(UM_GRAPOLLINGPROCESS);
						break;
					}
				// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				case  TESTCMD:
					{
						if (rType == 0x01)
						{
							m_OperDlg.SendMessage(UM_OPERSAVERAMDATA);
							m_OperDlg.OnBnClickedBtnOpeTeSavedata();
						}
						break;
					}
#if HIDNUM==1
				case GRACMD:
					{
						if (RxData[5] == 0xF1)
						{
							MessageBox("Error: Chip sample abnormally");
							PCRType = 0;
							memset(&RxData,0,sizeof(RxData));
							return;
						}
						else
						{
							PCRTypeFilterClass = rType & 0x0F;		// ÅÐ¶Ï·µ»ØµÄÀà±ð£¬ÊÇ»­ÐÐ¡¢Ò³»¹ÊÇvedio
							PCRTypeFilterNum = rType & 0xF0;		// ÅÐ¶ÏÊÇµÚ¼¸¸öPCR·µ»ØµÄÊý¾Ý
							//È¡ÏàÓ¦µÄÓÐÐ§Êý¾Ý
							if ((PCRTypeFilterClass==0x01)|(PCRTypeFilterClass==0x02)|(PCRTypeFilterClass==0x03) )		//·µ»Ø12 pixel Êý¾ÝÊ±
							{	
								// Çø·ÖÐòºÅ
								switch(PCRTypeFilterNum)
								{
								case 0x00:
									PCRNum = 1;
									break;
								case 0x10:
									PCRNum = 2;
									break;
								case 0x20:
									PCRNum = 3;
									break;
								case 0x30:
									PCRNum = 4;
									break;
								default:
									break;
								}

								if (RxData[5]==0x0b)		// HID¶Áµ½µÚ12ÐÐºóÍ£Ö¹¶ÁÈ¡
									Gra_pageFlag = false;
								else
									Gra_pageFlag = true;

								for(k=0; k<26; k++)
								{
									Valid_strtemp.Format("%02X",RxData[k+6]);	//Ê®Áù½øÖÆÏÔÊ¾×Ö·û´®buffer, Ã¿¸öbyte¼ä¼Ó¿Õ¸ñ
									Valid_DStrtemp.Format("%02d",RxData[k+6]);	//Ê®½øÖÆÏÔÊ¾×Ö·û´®buffer, Ã¿¸öbyte¼ä¼Ó¿Õ¸ñ

									Valid_RegRecStr += (Valid_strtemp+" ");
									Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
								}
								//		Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
								//		Valid_RegRecStr += "\r\n";
								//		Valid_Dec_RegRecStr += "\r\n";
							}
							else
							{
								if ((PCRTypeFilterClass==0x07)|(PCRTypeFilterClass==0x08)|(PCRTypeFilterClass==0x0b))	//·µ»Ø24 pixel Êý¾ÝÊ±
								{
									// Çø·ÖÐòºÅ
									switch(PCRTypeFilterNum)
									{
									case 0x00:
										PCRNum = 1;
										break;
									case 0x10:
										PCRNum = 2;
										break;
									case 0x20:
										PCRNum = 3;
										break;
									case 0x30:
										PCRNum = 4;
										break;
									default:
										break;
									}

									if (RxData[5]==0x17)	// HID¶Áµ½µÚ24ÐÐºóÍ£Ö¹¶ÁÈ¡
										Gra_pageFlag = false;
									else
										Gra_pageFlag = true;

									for(k=0; k<50; k++)
									{
										Valid_strtemp.Format("%02X",RxData[k+6]);	//Ê®Áù½øÖÆÏÔÊ¾×Ö·û´® buffer, Ã¿¸ö byte ¼ä¼Ó¿Õ¸ñ
										Valid_DStrtemp.Format("%02d",RxData[k+6]);	//Ê®½øÖÆÏÔÊ¾×Ö·û´® buffer, Ã¿¸ö byte ¼ä¼Ó¿Õ¸ñ

										Valid_RegRecStr += (Valid_strtemp+" ");
										Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
									}
									//Ã¿ÐÐÊý¾Ý¼ä¼ÓÈë»Ø³µ
									//						Valid_RegRecStr += "\r\n";
									//						Valid_Dec_RegRecStr += "\r\n";
								}
								else	//ÆäËütype´ý¶¨
								{
									Valid_RegRecStr.Empty();
									Valid_Dec_RegRecStr.Empty();
								}
							}

							//Æô¶¯Graphic dialogÏûÏ¢´¦Àí³ÌÐò
							m_GraDlg.SendMessage(UM_GRAPROCESS);
						}				
						break;
					}
				case POLLINGRACMD:
					{
						if (rType == 0x01)
						{
							BYTE rData = 0;
							rData = RxData[5] & 0x0F;

							m_TrimDlg.TrimGetPCRMaskStatus();

//							if (rData == PCRMask)		// pollingµ½ÓÐÐ§·µ»ØÊý¾Ý

							if (rData)		// pollingµ½ÓÐÐ§·µ»ØÊý¾Ý
							{
								if (rData != PCRMask)		// pollingµ½ÓÐÐ§·µ»ØÊý¾Ý
//								if(m_GraDlg.m_DelayTrigger)
									Sleep(1000);

								PollingGraTimerFlag = FALSE;	// ÔÝÍ£graphic HID polling

								CString str;
								str.Format("Stop polling Gra HID...(rData: %x)", rData);
								m_GraDlg.DisplayPollStatus(str);

/*								// ÏÔÊ¾graphic dialog 
								// m_GraDlg.ShowWindow(SW_SHOW);	
								CRect tabRect;    // ±êÇ©¿Ø¼þ¿Í»§ÇøµÄRect     
								// »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect£¬²¢¶ÔÆäµ÷Õû£¬ÒÔÊÊºÏ·ÅÖÃ±êÇ©Ò³     
								m_tab.GetClientRect(&tabRect);     
								tabRect.left += 1;     
								tabRect.right -= 1;     
								tabRect.top += 25;     
								tabRect.bottom -= 1; 

								m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
								m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
*/								// Zhimin: No need to switch windows.

								// ·¢ËÍÒ³Í¼ÏñÏÔÊ¾ÃüÁî
								m_GraDlg.CaptureFrame(true);			// Zhimin modified.
							}
						}
						break;
					}
				case  0x17:
				{
					if (rType == 0x01)
					{
						m_GraDlg.SendMessage(UM_GRACYCPOLLPROC);
					}
					break;
				}

#endif
				default:
					{
						break;
					}
				}
					
		break;
		}
	case WAIT_TIMEOUT:
		{
//			if (hidReadCnt == HIDREADCNTNUM)
//			{
//				hidReadCnt = 0;

				SetDlgItemText(IDC_STATICOpenComm, "Temperature HID ReadFile timeout");

				/*API call: CancelIo
				Cancels the ReadFile
				Requires the device handle.
				Returns non-zero on success.
				*/
		
				Result = CancelIo(Tem_ReadHandle);
		
				//A timeout may mean that the device has been removed. 
				//Close the device handles and set MyDeviceDetected = False 
				//so the next access attempt will search for the device.
				CloseHandles();
				MyDeviceDetected = FALSE;
				g_DeviceDetected = FALSE;

//			}else{		//³¬Ê±Î´ÊÕµ½´Ó»ú·µ»ØÓÐÐ§Êý¾Ý
				
//				timerCtrFlag = FALSE;	//ÔÝÍ£Í¼Ïñ°åºÍÎÂ¶È°åtimer·¢ËÍHIDÖ¸Áî
//				Sleep(200);				//µÈ´ý´Ó»úÏìÓ¦£¬Ê±¼ä¸ù¾ÝÏµÍ³Çé¿öÐÞ¸Ä

//				TEMHID_ReadHIDInputReport();	//HIDÔÙ´Î¶ÁÈ¡´Ó»úÊý¾Ý
//				hidReadCnt++;					//¶ÁÈ¡´ÎÊý¼ÆÊý
//			}
		
		break;
		}
	default:
		{
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.

		CloseHandles();
		SetDlgItemText(IDC_STATICOpenComm,"Can't read from Temperature HID device");
		MyDeviceDetected = FALSE;
		g_DeviceDetected = FALSE;
		break;
		}
	}

	/*
	API call: ResetEvent
	Sets the event object to non-signaled.
	Requires a handle to the event object.
	Returns non-zero on success.
	*/

	ResetEvent(TemHID_hEventObject);

	//Display the report data.
	DisplayInputReport();

}


void CPCRProjectDlg::RegisterForDeviceNotifications()
{

	// Request to receive messages when a device is attached or removed.
	// Also see WM_DEVICECHANGE in BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialog).

	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	HDEVNOTIFY DeviceNotificationHandle;

	DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
	DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	DevBroadcastDeviceInterface.dbcc_classguid = HidGuid;

	DeviceNotificationHandle =
		RegisterDeviceNotification(m_hWnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

}


//....................................................................


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
//	virtual void OnCancel();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPCRProjectDlg dialog


CPCRProjectDlg::CPCRProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPCRProjectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_StaticOpenComm = _T("");
	//  m_strBytesReceived = _T("");
	m_strBytesReceived = _T("");

	trim_loaded = false;
}

void CPCRProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_MSCOMM1, m_mscomm);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Text(pDX, IDC_STATICOpenComm, m_StaticOpenComm);
	//  DDX_Text(pDX, IDC_EDIT_HIDRECEIVE, m_strBytesReceived);
	//  DDX_Control(pDX, IDC_EDIT_HIDRECEIVE, m_BytesReceived);
	DDX_Control(pDX, IDC_lstBytesReceived, m_BytesReceived);
	DDX_LBString(pDX, IDC_lstBytesReceived, m_strBytesReceived);
}

/*void CPCRProjectDlg::OnOK()
{

}

void CPCRProjectDlg::OnCancel()
{
	int result = MessageBox("OK to close?", "ULVision", MB_ICONQUESTION | MB_OKCANCEL);

	if (result != IDOK) return;

	CDialogEx::OnCancel();
}
*/

BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CPCRProjectDlg::OnTcnSelchangeTab1)
	ON_MESSAGE(WM_RegDlg_event,OnRegDlg)
	ON_MESSAGE(WM_GraDlg_event,OnGraDlg)
	ON_MESSAGE(WM_TrimDlg_event,OnTrimDlg)
	ON_MESSAGE(WM_ReadHID_event,OnReadHID)
	ON_MESSAGE(WM_OperDlg_event,OnOperDlg)
	ON_MESSAGE(WM_ParsaveDlg_event,OnParsaveDlg)
	ON_BN_CLICKED(IDC_BTN_OPENCOMM, &CPCRProjectDlg::OnBnClickedBtnOpencomm)
	ON_BN_CLICKED(IDC_BTN_OPENHID, &CPCRProjectDlg::OnBnClickedBtnOpenhid)
	//....................................................................
	//ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_DEVICECHANGE, Main_OnDeviceChange)
	//....................................................................
	ON_BN_CLICKED(IDC_BTN_SENDHID, &CPCRProjectDlg::OnBnClickedBtnSendhid)
	ON_BN_CLICKED(IDC_BTN_READHID, &CPCRProjectDlg::OnBnClickedBtnReadhid)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_PCR_BTN_VER, &CPCRProjectDlg::OnBnClickedPcrBtnVer)
	ON_STN_CLICKED(IDC_PCR_STATIC_VER, &CPCRProjectDlg::OnStnClickedPcrStaticVer)
END_MESSAGE_MAP()


// CPCRProjectDlg message handlers

TCHAR g_CurrentDirectory[MAX_PATH];

BOOL CPCRProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	//=================

	IntTimeDlg  eidDlg;				// experiment ID
	CString expName, trimName, dpName, trimName2, trimName3, trimName4;
	BOOL  newproj = true;

	if (eidDlg.DoModal() == IDOK)			// factory version do not show load experiment box, treat as new proj also
	{
//		g_ChipID = cidDlg.m_ChipID;
//		g_curTemp = cidDlg.m_curTemp;

		trimName = eidDlg.m_trimName;
		trimName2 = eidDlg.m_trimName2;
		trimName3 = eidDlg.m_trimName3;
		trimName4 = eidDlg.m_trimName4;
		dpName = eidDlg.m_ExpName2;

	}
	/*else {
		PostMessage(WM_CLOSE);
	}*/

	g_ExpName = expName;


//	SetDlgItemText(IDC_STATIC_EID, "PCRProject (c) Anitoa - " + expName);

	SetWindowText("PCRProject (c) Anitoa - " + dpName + " + " + trimName);

	//============================

	//Initial TAB component

	CRect tabRect;

#if PAGENUM == 2
	//	m_tab.InsertItem(0, _T("Thermal Cycle"));			// 

	//added by lixin 20190327
	switch(currentLanguage){
		case language_English:
			m_tab.InsertItem(0, _T("Main control"));				// 
			m_tab.InsertItem(1, _T("Imager parameters"));
			m_tab.InsertItem(2, _T(("Cycler parameters")));
			break;
		case language_Chinese:
			m_tab.InsertItem(0, _T("主控界面"));				// 
			m_tab.InsertItem(1, _T("图像控制"));
			m_tab.InsertItem(2, _T(("温控界面")));
			break;
	}  
	
	//	m_RegDlg.Create(IDD_REGISTER_DIALOG, &m_tab);		//   
	m_GraDlg.Create(IDD_GRAPHIC_DIALOG, &m_tab);	//  
	m_TrimDlg.Create(IDD_TRIM_DIALOG, &m_tab);		//
	m_OperDlg.Create(IDD_OPERATION_DIALOG,&m_tab);

	m_tab.GetClientRect(&tabRect);    //   
	//  
	tabRect.left += 1;                    
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;     
	// ¸ù¾Ýµ÷ÕûºÃµÄtabRect·ÅÖÃm_RegDlg×Ó¶Ô»°¿ò£¬²¢ÉèÖÃÎªÏÔÊ¾     
	//	m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);     
	// ¸ù¾Ýµ÷ÕûºÃµÄtabRect·ÅÖÃm_GraDlg×Ó¶Ô»°¿ò£¬²¢ÉèÖÃÎªÒþ²Ø     
	m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);  // SWP_HIDEWINDOW);
	// ¸ù¾Ýµ÷ÕûºÃµÄtabRect·ÅÖÃm_TrimDlg×Ó¶Ô»°¿ò£¬²¢ÉèÖÃÎªÒþ²Ø     
	m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
	// ¸ù¾Ýµ÷ÕûºÃµÄtabRect·ÅÖÃm_OperDlg×Ó¶Ô»°¿ò£¬²¢ÉèÖÃÎªÒþ²Ø     
	m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
#endif

	// Ö»ÏÔÊ¾operation dialog
#if PAGENUM == 1
	m_tab.InsertItem(2,_T(("Operation")));
	m_OperDlg.Create(IDD_OPERATION_DIALOG,&m_tab);
	m_tab.GetClientRect(&tabRect);    // »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect     
	// µ÷ÕûtabRect£¬Ê¹Æä¸²¸Ç·¶Î§ÊÊºÏ·ÅÖÃ±êÇ©Ò³     
	tabRect.left += 1;                    
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;
	m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
#endif

//	Register to receive device notifications.
//	RegisterForDeviceNotifications();

//	if(FindTheHID()) m_TrimDlg.ResetTrim();
	//	FindTheHID();

	g_pTrimDlg = &m_TrimDlg;
	g_pGraDlg = &m_GraDlg;
	g_pTrimReader = &m_TrimReader;

	// Load Trim data

	GetCurrentDirectory(MAX_PATH, g_CurrentDirectory);

	CString path;

	if (trimName.IsEmpty()) {
		path = g_CurrentDirectory;
		path += "\\Trim\\trim.dat";
	}
	else {
		path = trimName;
	}

	LPTSTR lpszData = path.GetBuffer(path.GetLength());
	trim_loaded = m_TrimReader.Load((TCHAR*)lpszData);

	path.ReleaseBuffer(0);

	m_DPReader.regDispatch(dpName);
	m_DPReader.Parse();						// This will parse from the global variables from GraDlg, not from the data position file
	g_pDPReader = &m_DPReader;

	CString str, s;
	int sn1 = (int)m_DPReader.serial_number1;
	int sn2 = (int)m_DPReader.serial_number2;
	s = m_DPReader.id_str.c_str();

	if (m_DPReader.version == 0) {
		s = (char)m_DPReader.id;
		
		str.Format("%s-%d-%d", s, sn1, sn2);
	}
	else {
		// str = m_DPReader.id_str.c_str();
		str.Format("%s-%d-%d", s, sn1, sn2);
	}

	CString mess;
	
	//added by lixin 20190327
	switch(currentLanguage){
		case language_English:
			mess.Format("System ID: %s", str);
			break;
		case language_Chinese:
			mess.Format("系统编号: %s", str);
			break;
	} 

	test_report.sys_id = str;
	
	//added by lixin 20190327
	switch(currentLanguage){
		case language_English:
			mess += "   Trim loaded: ";
			break;
		case language_Chinese:
			mess += "   Trim 文件: ";
			break;
	} 

	if(trim_loaded) {
		m_TrimReader.Parse();

		path = trimName2;
		lpszData = path.GetBuffer(path.GetLength());
		m_TrimReader.Load((TCHAR*)lpszData);
		m_TrimReader.Parse(1);

		path = trimName3;
		lpszData = path.GetBuffer(path.GetLength());
		m_TrimReader.Load((TCHAR*)lpszData);
		m_TrimReader.Parse(2);

		path = trimName4;
		lpszData = path.GetBuffer(path.GetLength());
		m_TrimReader.Load((TCHAR*)lpszData);
		m_TrimReader.Parse(3);		
		
		path.ReleaseBuffer(0);

		mess += m_TrimReader.Node[0].name + " ";
		mess += m_TrimReader.Node[1].name + " ";
		mess += m_TrimReader.Node[2].name + " ";
		mess += m_TrimReader.Node[3].name;
	}
	else {
		//added by lixin 20190327
		switch(currentLanguage){
			case language_English:
				mess += "Device Trim Data Not Loaded";
				break;
			case language_Chinese:
				mess += "Trim 文件没有装上";
				break;
		} 
	}	
		
	SetDlgItemText(IDC_STATIC_TRIM, mess);

	bool fh = FindTheHID();
	if(fh) {
		m_TrimDlg.ResetTrim();
		m_OperDlg.ResetParam();
	}
	else {
		RegisterForDeviceNotifications();
	}

//	m_OperDlg.OnBnClickedButtonLdjson();

	m_OperDlg.Loadjson(expName, newproj);


	if (!trim_loaded && fh) {
//		CString s = m_TrimDlg.ReadFlash();
//		SetDlgItemText(IDC_STATIC_TRIM, s);

		int result = MessageBox("Read trim data from Flash?", "PCRProject", MB_ICONQUESTION | MB_YESNO);
		if (result == IDYES) {
			CString s = m_TrimDlg.ReadFlash();
			CString str;

			switch (currentLanguage) {
			case language_English:
				str = "System ID: " + s;
				break;
			case language_Chinese:
				str = "系统编号: " + s;
				break;
			}

			SetDlgItemText(IDC_STATIC_TRIM, str);

			test_report.sys_id = s;
		}
		else {
			MessageBox("Without device trim data, system will not operate properly", "PCRProject", MB_ICONERROR);
		}
	}

	CTime systime = CTime::GetCurrentTime();
	CString tstr;

	tstr.Format("%4d-%.2d-%.2d_%.2d:%.2d:%.2d",
		systime.GetYear(), systime.GetMonth(), systime.GetDay(),
		systime.GetHour(), systime.GetMinute(), systime.GetSecond());

	test_report.time = tstr;

	test_report.config.Format("%d-%d", g_pDPReader->num_wells, g_pDPReader->num_channels);

//	m_GraDlg.DisplayReportStatus("ÏµÍ³±àºÅ: " + test_report.sys_id + " (" + test_report.config + ")\r\n" + test_report.time);

	OnBnClickedPcrBtnVer();

	switch (currentLanguage) {
		case language_English:
			m_GraDlg.DisplayReportStatus("System ID: " + test_report.sys_id + "\r\nTest Time: " + test_report.time + "\r\nNum of Wells and Channels: " + test_report.config);
			m_GraDlg.DisplayReportStatus("FW Versions: \r\n" + sTemFirmwareVer);
			break;
		case language_Chinese:
			m_GraDlg.DisplayReportStatus("System ID (系统编号)： " + test_report.sys_id + "\r\nTest Time (测试时间)： " + test_report.time + "\r\nNum of Wells and Channels(反应池及通道)： " + test_report.config);
			m_GraDlg.DisplayReportStatus("FW Versions (系统下位机版本编号)：\r\n" + sTemFirmwareVer);
			break;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCRProjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPCRProjectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPCRProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BEGIN_EVENTSINK_MAP(CPCRProjectDlg, CDialogEx)
	ON_EVENT(CPCRProjectDlg, IDC_MSCOMM1, 1, CPCRProjectDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()


void CPCRProjectDlg::OnCommMscomm1()
{
}

LRESULT CPCRProjectDlg::OnReadHID(WPARAM wParam, LPARAM lParam)
{
	GRAHID_ReadHIDInputReport();

	return 0;
}

LRESULT CPCRProjectDlg::OnRegDlg(WPARAM wParam, LPARAM lParam)
{
	switch(RegFlag)
	{

	case sendregmsg:
		{
			// ÏòHID·¢ËÍÊý¾Ý
			GRAHID_WriteHIDOutputReport();		

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			RegFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(RegBuf,0,sizeof(RegBuf));
			
			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
			SendHIDRead();			

			break;
		}
	default:
		AfxMessageBox(_T("Please Send Register Flag Correctly"));
		break;

	}

	return 0;
}

LRESULT CPCRProjectDlg::OnGraDlg(WPARAM wParam, LPARAM lParam)
{
	switch(GraFlag)
	{
	case SENDGRAMSG:
		{
			// ÏòHID·¢ËÍÊý¾Ý
#if HIDNUM==2			
			GRAHID_WriteHIDOutputReport();	
#endif
#if HIDNUM==1
			TEMHID_WriteHIDOutputReport();
#endif				

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));
			memset(RxData,0,sizeof(RxData));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
												// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
												// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
			break;
		}
	case SENDPAGEMSG:
		{
			// ÏòHID·¢ËÍÊý¾Ý
#if HIDNUM==2			
			GRAHID_WriteHIDOutputReport();	
#endif
#if HIDNUM==1
			TEMHID_WriteHIDOutputReport();
#endif	
			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));
			memset(RxData,0,sizeof(RxData));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
			// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
			// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
			break;
		}
	case READGRAMSG:
		{
			GraFlag = 0;
			memset(RxData,0,sizeof(RxData));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
			// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
			// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
		}
	case SENDVIDEOMSG:
		{
			break;
		}
	case STARTTEMHID:	// ²éÑ¯temper HIDÊÇ·ñ¼ÌÐøpolling
		{
			GraFlag = 0;
			WPARAM a = 8;
			LPARAM b = 9;
			m_GraDlg.DisplayPollStatus("Poll cycler status...(one shot)");
			m_OperDlg.OnOpearCheckPollingProcess(a,b);

			break;
		}
	case STARTTHERMCYC:	// 
	{
		GraFlag = 0;
		m_TrimDlg.CommitTrim();
		m_OperDlg.OnBnClickedBtnOpeStart();
		break;
	}
	case STOPTHERMCYC:	// 
	{
		GraFlag = 0;
		m_OperDlg.OnBnClickedBtnOpeStop();
		break;
	}	
	case GETPCRMASK:
	{
		GraFlag = 0;
		m_TrimDlg.TrimGetPCRMaskStatus();
		break;
	}
	case READSTATUS:	// Zhimin added
	{
#if HIDNUM==1
		TEMHID_WriteHIDOutputReport();
#endif				
		GraFlag = 0;
		memset(TxData, 0, sizeof(TxData));
		memset(RxData, 0, sizeof(RxData));

#if HIDNUM==1
		TEMHID_ReadHIDInputReport();
#endif
		break;
	}
	default:
		AfxMessageBox(_T("Please Send Graphic Flag Correctly"));
		break;

	}

	return 0;
}

extern BOOL ee_continue;

LRESULT CPCRProjectDlg::OnTrimDlg(WPARAM wParam, LPARAM lParam)
{
	switch(TrimFlag)
	{
	case SENDTRIMMSG:
		{
//			CommSend(dNum);		//Ïò´®¿Ú·¢ËÍÊý¾Ý
			// ÏòHID·¢ËÍÊý¾Ý
#if HIDNUM==2			
			GRAHID_WriteHIDOutputReport();	
#endif
#if HIDNUM==1
			TEMHID_WriteHIDOutputReport();
#endif

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			TrimFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(TrimBuf,0,sizeof(TrimBuf));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
			// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
			// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
			break;
		}
	case TRIM_TWO_HID:
		{
			// ÏòHID·¢ËÍÊý¾Ý
#if HIDNUM==2			
			GRAHID_WriteHIDOutputReport();	
#endif
#if HIDNUM==1
			TEMHID_WriteHIDOutputReport();
#endif

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			TrimFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(TrimBuf,0,sizeof(TrimBuf));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
			// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
			// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
			break;
		}
	case EEPROMMSG:
		{
			TEMHID_WriteHIDOutputReport();		//

			TrimFlag = 0;
			memset(TxData, 0, sizeof(TxData));
			memset(TrimBuf, 0, sizeof(TrimBuf));

			TEMHID_ReadHIDInputReport();

			while (ee_continue) {			// read 4 packets
				TEMHID_ReadHIDInputReport();
			}

		break;
		}
	default:
		AfxMessageBox(_T("Please Send Trim Flag Correctly"));
		break;

	}

	return 0;
}


LRESULT CPCRProjectDlg::OnOperDlg(WPARAM wParam, LPARAM iParam)
{
	switch(OperDlgFlag)
	{
	case OPEREADSTATUS:
		{
			TEMHID_WriteHIDOutputReport();		// ÏòHID·¢ËÍÊý¾Ý

			if (TxData[1] == 0x13 && TxData[3] == 0x04) {
				int jk = 0;
			}

			if (TxData[1] == 0x13 && TxData[3] == 0x09) {
				int jk = 0;
			}

			rCmd =TxData[1];	//È¡³öÏÂÎ»»ú·µ»ØµÄÃüÁî
			rType = TxData[3];	//È¡³öÏÂÎ»»ú·µ»ØµÄtype

			CString strtest;

			if (rEcode != 0 || rCmd == 0x13) {
				strtest.Format("Sent-Cmd: %2x, Type: %2x", rCmd, rType);
				m_GraDlg.DisplayPollStatus(strtest);
			}

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			OperDlgFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(OperBuf,0,sizeof(OperBuf));

//			memset(RxData,0,sizeof(RxData));
			TEMHID_ReadHIDInputReport();		// ¶ÁÈ¡HID·µ»ØÊý¾Ý
			break;
		}
	case OPESENDMSG:
		{
			TEMHID_WriteHIDOutputReport();		// ÏòHID·¢ËÍÊý¾Ý

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			OperDlgFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(OperBuf,0,sizeof(OperBuf));
			break;
		}
	case OPETOGRAMSG:
		{
			// Ïògraphic HID·¢ËÍÊý¾Ý
#if HIDNUM==2			
			GRAHID_WriteHIDOutputReport();	
#endif
#if HIDNUM==1
			TEMHID_WriteHIDOutputReport();
#endif

			//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
			OperDlgFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(OperBuf,0,sizeof(OperBuf));

			// ¶ÁÈ¡HID·µ»ØÊý¾Ý
#if HIDNUM==2
			GRAHID_ReadHIDInputReport();		// ¿ÉÄÜÊÇ³¤Ê±¼ä½ÓÊÕÊ±¶ª°üµÄÔ­Òò
			// ÏòHIDÐ´ÍêºóÔÙ¶ÁµÄ¼ä¸ôÌ«¶Ì
			// ¿ÉÒÔºÍÏÂÎ»»úÔö¼Ó·À¶ª°ü»úÖÆ£¬ÅÐ¶Ï½ÓÊÕÍê³ÉÏÂÎ»»úÔÙ·¢ÏÂÒ»´ÎÊý¾Ý
#endif
#if HIDNUM==1
			TEMHID_ReadHIDInputReport();
#endif
			break;
		}
	case OPEGETPCRMASK:
		{
		OperDlgFlag = 0;
		m_TrimDlg.TrimGetPCRMaskStatus();
		break;
		}
	default:
		AfxMessageBox(_T("Please Send Graphic Flag Correctly"));
		break;

	}
	return 0;
}


LRESULT CPCRProjectDlg::OnParsaveDlg(WPARAM wParam, LPARAM iParam)
{
	CString sTemp = "builder:";
	if (sFileName != "")
	{
//		sBuilderName += (sTemp + "\r\n");
		sBuilderName += (sTemp + " ");
		sBuilderName += sSave;
		sBuilderName += "\0";
		sFileName += ".txt";
		CFile m_statusFile(sFileName,CFile::modeCreate|CFile::modeWrite);
		m_statusFile.Write(sBuilderName,sBuilderName.GetLength());
	}
	else
		AfxMessageBox("Please fill the filename");

	return 0;
}


void CPCRProjectDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CRect tabRect;    // ±êÇ©¿Ø¼þ¿Í»§ÇøµÄRect     

	// »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect£¬²¢¶ÔÆäµ÷Õû£¬ÒÔÊÊºÏ·ÅÖÃ±êÇ©Ò³     
	m_tab.GetClientRect(&tabRect);     
	tabRect.left += 1;     
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1; 


#if PAGENUM == 2
	switch (m_tab.GetCurSel())     
	{     
		
		// Èç¹û±êÇ©¿Ø¼þµ±Ç°Ñ¡Ôñ±êÇ©Îª¡°register¡±£¬ÔòÏÔÊ¾m_regdlg¶Ô»°¿ò£¬Òþ²Øm_gradlg¶Ô»°¿ò     
/*	case 0:     
		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;     
		// Èç¹û±êÇ©¿Ø¼þµ±Ç°Ñ¡Ôñ±êÇ©Îª¡°graphic¡±£¬ÔòÒþ²Øm_regdlg¶Ô»°¿ò£¬ÏÔÊ¾m_gradlg¶Ô»°¿ò     
*/	case 0:     
//		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW); 
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;
	case 1:
//		m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;
	case 2:   
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		break;
	default:     
		break;     
	}
#endif

#if PAGENUM == 1
	switch (m_tab.GetCurSel())     
	{     
		case 0:     
			m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		break;
		default:     
		break;     
	}
#endif
	
}

void CPCRProjectDlg::GetCom()
{
/*	HANDLE  hCom;
	int i;
	CString str;
	BOOL flag;

	flag = FALSE;	//´®¿ÚÊÇ·ñÕÒµ½±êÖ¾

	for (i = 1;i <= 16;i++)
	{//´Ë³ÌÐòÖ§³Ö16¸ö´®¿Ú£¬´Ó1µ½16ÒÀ´Î²éÕÒ

		str.Format(_T("\\\\.\\COM%d"),i);		//ÉèÖÃÒª²éÕÒµÄ´®¿ÚµØÖ·

		hCom = CreateFile(str, 0, 0, 0, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);	//´ò¿ªÖ¸¶¨µØÖ·µÄ´®¿Ú
		if(INVALID_HANDLE_VALUE != hCom )
		{//ÄÜ´ò¿ª¸Ã´®¿Ú£¬ÔòÌí¼Ó¸Ã´®¿Ú
			CloseHandle(hCom);
			str = str.Mid(4);		//ÒÔ×Ö·û´®ÐÎÊ½È¡³ö´®¿ÚµÄÃû×ÖCOMX
			m_StaticOpenComm = str;	//½«È¡µ½µÄ´®¿Ú×Ö·û´®Ãû×Ö¸³Öµ¸øÎÄ±¾¿Ø¼þ
			flag = TRUE;
		}
		else
			{//²»ÄÜ´ò¿ª½«´ò¿ª±êÊ¾Éè³Éfalse
				if (flag == TRUE)
					flag = TRUE;
				else
					flag = FALSE;
			}
	}

	if (flag == TRUE)	//ÕÒµ½¶Ë¿Ú
	{
		CString str1;

		str1 = m_StaticOpenComm.GetAt(3);	//½«´®¿Ú×Ö·û´®Ãû×Ö£¨COMX£©ÖÐµÄÊý×ÖXÈ¡³ö
		pnum = atoi(str1);	//×ª³ÉÊ®½øÖÆÊýÖµ

//		m_StaticOpenComm = _T("Connect ") + m_StaticOpenComm + str1;
//		UpdateData(FALSE);
	}
	else	//Ã»ÕÒµ½¶Ë¿Ú
		AfxMessageBox(_T("Can't find the comm port"));	
*/
}

// Repurposed to save configure

void CPCRProjectDlg::OnBnClickedBtnOpencomm()
{
	// TODO: Add your control notification handler code here
/*
	m_StaticOpenComm="";
	GetCom();

	if (m_mscomm.get_PortOpen())  
		m_mscomm.put_PortOpen(FALSE); //¹Ø±Õ´®¿Ú  

	m_mscomm.put_CommPort(pnum);     //Éè¶¨´®¿ÚÎªCOM4  


	m_mscomm.put_Settings(_T("9600,n,8,1"));  //Éè¶¨²¨ÌØÂÊ9600£¬ÎÞÆæÅ¼Ð£Ñé£¬8Î»Êý¾ÝÎ»£¬1Î»Í£Ö¹Î»  
	m_mscomm.put_InputMode(1);    //Éè¶¨Êý¾Ý½ÓÊÕÄ£Ê½£¬1Îª¶þ½øÖÆ£¬0ÎªÎÄ±¾  
	m_mscomm.put_InputLen(ONCOMNUM);     //Éè¶¨µ±Ç°½ÓÊÕÇøÊý¾Ý³¤¶È£¬0±íÊ¾È«²¿¶ÁÈ¡  
	m_mscomm.put_InBufferSize(1024);  //Éè¶¨ÊäÈë»º³åÇø´óÐ¡Îª1024 byte  
	m_mscomm.put_OutBufferSize(1024); //Éè¶¨Êä³ö»º³åÇø´óÐ¡Îª1024 byte  
	m_mscomm.put_RThreshold(ONCOMNUM);   //Ã¿½ÓÊÕµ½59¸ö×Ö·ûÊ±£¬´¥·¢OnCommÊÂ¼þ  
	m_mscomm.put_SThreshold(0);   //Éè¶¨Ã¿·¢ËÍ¶àÉÙ¸ö×Ö·û´¥·¢OnCommÊÂ¼þ£¬0±íÊ¾²»´¥·¢OnCommÊÂ¼þ  

	if (!m_mscomm.get_PortOpen())  
	{
		m_mscomm.put_PortOpen(TRUE);  //´ò¿ª´®¿Ú 
		m_StaticOpenComm = _T("Connect the ") + m_StaticOpenComm;
		UpdateData(FALSE);
	}
	else  
		AfxMessageBox(_T("can't open the comm port"));
*/
	CFileDialog saveDlg(FALSE, ".json",
		g_ExpName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"JSON(*.json)|*.json|""TXT(*.txt)|*.txt|""All Files(*.*)|*.*||",
		NULL, 0, TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// Áí´æ¶Ô»°¿òÄ¬ÈÏÂ·¾¶ÎªcÅÌ

	// ½øÐÐ±£´æ¶¯×÷
	if (saveDlg.DoModal() == IDOK)
	{
		CString fname = saveDlg.GetPathName();   // 
		m_OperDlg.Savejson(fname, true);

		g_dirty = false;
	}

}


void CPCRProjectDlg::OnBnClickedBtnOpenhid()
{
	// TODO: Add your control notification handler code here
//	if(CPCRProjectDlg::FindTheHID()) m_TrimDlg.ResetTrim();
	if (FindTheHID()) {
		m_TrimDlg.ResetTrim();
		m_OperDlg.ResetParam();
	}

//	assert(0);
}


void CPCRProjectDlg::OnBnClickedBtnSendhid()
{
	// TODO: Add your control notification handler code here
	CRect tabRect;    // ±êÇ©¿Ø¼þ¿Í»§ÇøµÄRect     
	// »ñÈ¡±êÇ©¿Ø¼þ¿Í»§ÇøRect£¬²¢¶ÔÆäµ÷Õû£¬ÒÔÊÊºÏ·ÅÖÃ±êÇ©Ò³     
	m_tab.GetClientRect(&tabRect);     
	tabRect.left += 1;     
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1; 

	m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
	m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
	m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);

}


void CPCRProjectDlg::OnBnClickedBtnReadhid()
{
	// TODO: Add your control notification handler code here
}


void CPCRProjectDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

	//³õÊ¼»¯¶Á±¨¸æÊ±ÓÃµÄOverlapped½á¹¹Ìå
	//Æ«ÒÆÁ¿ÉèÖÃÎª0
	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	//´´½¨Ò»¸öÊÂ¼þ£¬Ìá¹©¸øReadFileÊ¹ÓÃ£¬µ±ReadFileÍê³ÉÊ±£¬
	//»áÉèÖÃ¸ÃÊÂ¼þÎª´¥·¢×´Ì¬¡£
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//´´½¨Ò»¸ö¶Á±¨¸æµÄÏß³Ì£¨´¦ÓÚ¹ÒÆð×´Ì¬£©
	pReadReportThread=AfxBeginThread(ReadReportThread,
		this,
		THREAD_PRIORITY_NORMAL,
		0,
		CREATE_SUSPENDED,
		NULL);
	//Èç¹û´´½¨³É¹¦£¬Ôò»Ö¸´¸ÃÏß³ÌµÄÔËÐÐ
	if(pReadReportThread!=NULL)
	{
		pReadReportThread->ResumeThread();
	}
}


void CPCRProjectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case 1:		// ³ÖÐøvideoÃüÁî·¢ËÍ¡¢Êý¾Ý¶ÁÈ¡
		{
/*			for (PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)		// 4¸öPCRÒÀ´Î·¢ËÍ
			{
				if (Vedio12_24Flag == 0)		// 12ÐÐvedio
				{
					PCRType = (PCRCycCnt<<4)|(0x02);		// ·Ö±ðÉèÖÃ4¸öPCRµÄtype
					m_GraDlg.MakeGraPacket(0x02,PCRType,0xff);

					GRAHID_WriteHIDOutputReport();		// ÏòHID·¢ËÍÊý¾Ý

					//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
					GraFlag = 0;
					//				memset(TxData,0,sizeof(TxData));
					//				memset(GraBuf,0,sizeof(GraBuf));

					while(Gra_pageFlag)
						GRAHID_ReadHIDInputReport();
					Gra_pageFlag = true;
				}
				else		// 24ÐÐvedio
				{
					PCRType = (PCRCycCnt<<4)|(0x08);		// ·Ö±ðÉèÖÃ4¸öPCRµÄtype
					m_GraDlg.MakeGraPacket(0x02,PCRType,0xff);

					GRAHID_WriteHIDOutputReport();		// ÏòHID·¢ËÍÊý¾Ý

					//ÃüÁî±êÖ¾¡¢´«ÊäbufferÇåÁã
					GraFlag = 0;
					//				memset(TxData,0,sizeof(TxData));
					//				memset(GraBuf,0,sizeof(GraBuf));

					while(Gra_pageFlag)
						GRAHID_ReadHIDInputReport();
					Gra_pageFlag = true;
				}
			}
			break;
*/		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CPCRProjectDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	bool factory_version = true;

	if (g_dirty && !factory_version) {
		int result = MessageBox("Save data before closing?", "PCRProject", MB_ICONQUESTION | MB_YESNO);
		if (result == IDOK) {
			OnBnClickedBtnOpencomm();
			return;
		}
	}

	int result = MessageBox("OK to close?", "PCRProject", MB_ICONQUESTION | MB_OKCANCEL);

	if (result != IDOK) return;

	EndDialog(IDCANCEL);
	CDialogEx::OnClose();
}


void CPCRProjectDlg::OnBnClickedPcrBtnVer()
{
	// TODO: Add your control notification handler code here
//	CDialog *pDlg = new CDialog;
//	pDlg -> Create(IDD_VERSION_DIALOG);

	// read HID firmware version
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x27;		//data type
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

	// read temperature HID firmware version
	if (TemHidFlag)
	{
		TEMHID_WriteHIDOutputReport();		// Ïòtemperatrue HID·¢ËÍÊý¾Ý

		TEMHID_ReadHIDInputReport();		// ¶ÁÈ¡temperatrue HID·µ»ØÊý¾Ý

//		pDlg -> SetDlgItemText(IDC_VERDLG_STATIC_TEMHIDVER,sTemFirmwareVer);
	}
	else
	{
		sTemFirmwareVer = "Temperature HID isn't connected";
//		pDlg -> SetDlgItemText(IDC_VERDLG_STATIC_TEMHIDVER,sTemFirmwareVer);
	}

	// read graphic HID firmware version
/*	if (GraHidFlag)
	{
		GRAHID_WriteHIDOutputReport();		// Ïògraphic HID·¢ËÍÊý¾Ý

		GRAHID_ReadHIDInputReport();		// ¶Ágraphic È¡HID·µ»ØÊý¾Ý

		pDlg -> SetDlgItemText(IDC_VERDLG_STATIC_GRAHIDVER,sGraFirmwareVer);
	}
	else
	{
		sGraFirmwareVer = "Graphic HID isn't connected";
		pDlg -> SetDlgItemText(IDC_VERDLG_STATIC_GRAHIDVER,sGraFirmwareVer);
	}
*/

	memset(TxData,0,sizeof(TxData));	//´«ÊäbufferÇåÁã

	// show UI version
	sUIVer.Empty();
	sUIVer.Format("UI Version is:  %.4d - %.2d - %.2d ",VERSIONYEAR,VERSIONMONTH,VERSIONDAY);
	
//	pDlg -> ShowWindow(SW_NORMAL);
//	pDlg -> SetDlgItemText(IDC_VERDLG_STATIC_VER,sUIVer);
	
}


void CPCRProjectDlg::OnStnClickedPcrStaticVer()
{
	// TODO: Add your control notification handler code here
}

