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

extern BYTE OperBuf[200];				// operation dialog buffer

//***************************************************************
//Global variable definition
//***************************************************************

void COperationDlg::SetPCRMask(int mask)
{
	OperDlgFlag = OPETOGRAMSG;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x24;		//data type
	TxData[4] = mask;
	TxData[5] = 0x00;
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	for (int i = 1; i < 8; i++)
		TxData[8] += TxData[i];
	if (TxData[8] == 0x17)
		TxData[8] = 0x18;
	else
		TxData[8] = TxData[8];
	TxData[9] = 0x17;
	TxData[10] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//
}

void COperationDlg::SetPiTemp(float temp)
{
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&temp;	//

	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = 0x01;		//real data
	TxData[5] = OperBuf[0];	//tp第一字节				
	TxData[6] = OperBuf[1];
	TxData[7] = OperBuf[2];
	TxData[8] = OperBuf[3];	//tp最后一字节
	TxData[9] = 0x00;		//time低字节
	TxData[10] = 0x00;		//time高字节
	TxData[11] = 0x00;		//预留位
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
	OperCalMainMsg();	//
}

// 13 03 3 segment command

void COperationDlg::SetPCRCyclTempTime(float den_temp, int den_time, float ann_temp, int ann_time, float ext_temp, int ext_time) {

	// third send
	OperDlgFlag = OPEREADSTATUS;

//	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM, stempture);		//取Dennature编辑框中的数据
//	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME, stime);

//	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

//	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

//	GetDlgItemText(IDC_EDIT_OPE_ANNTEM, stempture);		//取Anneal编辑框中的数据
//	GetDlgItemText(IDC_EDIT_OPE_ANNTIME, stime);

//	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ann_temp;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

//	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = ann_time >> 8;
	OperBuf[11] = ann_time;

//	GetDlgItemText(IDC_EDIT_OPE_EXTTEM, stempture);		//取Inter Extension编辑框中的数据
//	GetDlgItemText(IDC_EDIT_OPE_EXTTIME, stime);

//	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ext_temp;
	OperBuf[12] = hData[0];
	OperBuf[13] = hData[1];
	OperBuf[14] = hData[2];
	OperBuf[15] = hData[3];

//	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[16] = ext_time >> 8;
	OperBuf[17] = ext_time;

//	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE, stime);
//	itime = atoi(stime);
//	OperBuf[18] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x16;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte TXC
							//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;		//RegBuf[18];						
	TxData[5] = 0x01;        //RegBuf[20];	
	TxData[6] = 0x03;       // RegBuf[21];
	TxData[7] = OperBuf[0];		//dennature数据
	TxData[8] = OperBuf[1];
	TxData[9] = OperBuf[2];
	TxData[10] = OperBuf[3];
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	TxData[19] = OperBuf[12];	//Inter extension数据
	TxData[20] = OperBuf[13];
	TxData[21] = OperBuf[14];
	TxData[22] = OperBuf[15];
	TxData[23] = OperBuf[16];
	TxData[24] = OperBuf[17];
	for (int i = 1; i < 25; i++)
		TxData[25] += TxData[i];
	if (TxData[25] == 0x17)
		TxData[25] = 0x18;
	else
		TxData[25] = TxData[25];
	TxData[26] = 0x17;
	TxData[27] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}

// 13 03 2 seg

void COperationDlg::SetPCRCyclTempTime2Seg(float den_temp, int den_time, float ann_temp, int ann_time) 
{
	// third send
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

	hData = (unsigned char *)&ann_temp;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	OperBuf[10] = ann_time >> 8;
	OperBuf[11] = ann_time;

	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x13;			//command  TXC
	TxData[2] = 0x10;			//data length
	TxData[3] = 0x03;			//data type, date edit first byte TXC
	TxData[4] = 0x03;			//					
	TxData[5] = 0x01;			//	
	TxData[6] = 0x02;			// 
	TxData[7] = OperBuf[0];			//dennature
	TxData[8] = OperBuf[1];
	TxData[9] = OperBuf[2];
	TxData[10] = OperBuf[3];
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];

	for (int i = 1; i < 19; i++)
		TxData[19] += TxData[i];
	if (TxData[19] == 0x17)
		TxData[19] = 0x18;
//	else
//		TxData[19] = TxData[19];
	TxData[20] = 0x17;
	TxData[21] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();			//
}


// 13 04 set initial denature and hold time and temp

void COperationDlg::SetPCRCyclTempTime2(float iden_temp, int iden_time, float hold_temp, int hold_time, int num_cycle) {

	// forth send
	OperDlgFlag = OPEREADSTATUS;

	//GetDlgItemText(IDC_EDIT_OPE_DENTEM, stempture);		//取Initail denaturation编辑框中的数据
	//GetDlgItemText(IDC_EDIT_OPE_DENTIME, stime);

	//ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制

	unsigned char * hData = (unsigned char *)&iden_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	//itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = iden_time >> 8;
	OperBuf[5] = iden_time;

	//GetDlgItemText(IDC_EDIT_OPE_HOLDTEM, stempture);		//取Initail denaturation编辑框中的数据
	//GetDlgItemText(IDC_EDIT_OPE_HOLDTIME, stime);

	//ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&hold_temp;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	//itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = hold_time >> 8;
	OperBuf[11] = hold_time;

	//GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE, stime);
	//itime = atoi(stime);
	//TemperCycNum = itime;

	OperBuf[12] = num_cycle;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x10;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte TXC
	TxData[4] = 0x01;		//real data, close					// Zhimin comment: This byte determines that it is a start...
	TxData[5] = OperBuf[12];	//cycle setting
	TxData[6] = 0x00;       //
	TxData[7] = OperBuf[0];	//inital dennature数据	
	TxData[8] = OperBuf[1];  //
	TxData[9] = OperBuf[2];
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
	for (int i = 1; i < 19; i++)
		TxData[19] += TxData[i];
	if (TxData[19] == 0x17)
		TxData[19] = 0x18;
	else
		TxData[19] = TxData[19];
	TxData[20] = 0x17;
	TxData[21] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}

//===========New protocol commands=================
// These are "13 09" commands that allow specification of ramp rates

// 3 segment version

void COperationDlg::SetPCRCyclTempTimeNew(float den_temp, int den_time, float den_rate, float ann_temp, int ann_time, float ann_rate, float ext_temp, int ext_time, float ext_rate) 
{
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

	hData = (unsigned char *)&den_rate;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];


	hData = (unsigned char *)&ann_temp;
	OperBuf[10] = hData[0];
	OperBuf[11] = hData[1];
	OperBuf[12] = hData[2];
	OperBuf[13] = hData[3];

	OperBuf[14] = ann_time >> 8;
	OperBuf[15] = ann_time;

	hData = (unsigned char *)&ann_rate;
	OperBuf[16] = hData[0];
	OperBuf[17] = hData[1];
	OperBuf[18] = hData[2];
	OperBuf[19] = hData[3];


	hData = (unsigned char *)&ext_temp;
	OperBuf[20] = hData[0];
	OperBuf[21] = hData[1];
	OperBuf[22] = hData[2];
	OperBuf[23] = hData[3];

	OperBuf[24] = ext_time >> 8;
	OperBuf[25] = ext_time;

	hData = (unsigned char *)&ext_rate;
	OperBuf[26] = hData[0];
	OperBuf[27] = hData[1];
	OperBuf[28] = hData[2];
	OperBuf[29] = hData[3];


	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x13;			//command  TXC
	TxData[2] = 0x22;			//data length
	TxData[3] = 0x09;			//data type, date edit first byte TXC
								//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;			//RegBuf[18];						
	TxData[5] = 0x01;			//RegBuf[20];	
	TxData[6] = 0x03;			// RegBuf[21];
	TxData[7] = OperBuf[0];		//dennature数据
	TxData[8] = OperBuf[1];
	TxData[9] = OperBuf[2];
	TxData[10] = OperBuf[3];
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	TxData[19] = OperBuf[12];		// extension数据
	TxData[20] = OperBuf[13];
	TxData[21] = OperBuf[14];
	TxData[22] = OperBuf[15];
	TxData[23] = OperBuf[16];
	TxData[24] = OperBuf[17];
	TxData[25] = OperBuf[18];
	TxData[26] = OperBuf[19];
	TxData[27] = OperBuf[20];
	TxData[28] = OperBuf[21];
	TxData[29] = OperBuf[22];		// extension数据
	TxData[30] = OperBuf[23];
	TxData[31] = OperBuf[24];
	TxData[32] = OperBuf[25];
	TxData[33] = OperBuf[26];
	TxData[34] = OperBuf[27];
	TxData[35] = OperBuf[28];
	TxData[36] = OperBuf[29];

	for (int i = 1; i < 37; i++) {
		TxData[37] += TxData[i];
	}

	if (TxData[37] == 0x17)
		TxData[37] = 0x18;
//	else
//		TxData[38] = TxData[38];

	TxData[38] = 0x17;
	TxData[39] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//
}

//=============================

void COperationDlg::SetPCRCyclTempTime4Seg(float den_temp, int den_time, float den_ramp, float ann_temp, int ann_time, float ann_ramp, float ann2_temp, int ann2_time, float ann2_ramp, float ext_temp, int ext_time, float ext_ramp)
{
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

	hData = (unsigned char *)&den_ramp;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	hData = (unsigned char *)&ann_temp;
	OperBuf[10] = hData[0];
	OperBuf[11] = hData[1];
	OperBuf[12] = hData[2];
	OperBuf[13] = hData[3];

	OperBuf[14] = ann_time >> 8;
	OperBuf[15] = ann_time;

	hData = (unsigned char *)&ann_ramp;
	OperBuf[16] = hData[0];
	OperBuf[17] = hData[1];
	OperBuf[18] = hData[2];
	OperBuf[19] = hData[3];

	hData = (unsigned char *)&ann2_temp;
	OperBuf[20] = hData[0];
	OperBuf[21] = hData[1];
	OperBuf[22] = hData[2];
	OperBuf[23] = hData[3];

	OperBuf[24] = ann2_time >> 8;
	OperBuf[25] = ann2_time;

	hData = (unsigned char *)&ann2_ramp;
	OperBuf[26] = hData[0];
	OperBuf[27] = hData[1];
	OperBuf[28] = hData[2];
	OperBuf[29] = hData[3];

	hData = (unsigned char *)&ext_temp;
	OperBuf[30] = hData[0];
	OperBuf[31] = hData[1];
	OperBuf[32] = hData[2];
	OperBuf[33] = hData[3];

	OperBuf[34] = ext_time >> 8;
	OperBuf[35] = ext_time;

	hData = (unsigned char *)&ext_ramp;
	OperBuf[36] = hData[0];
	OperBuf[37] = hData[1];
	OperBuf[38] = hData[2];
	OperBuf[39] = hData[3];


	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x16;		//data length
	TxData[3] = 0x09;		//data type, date edit first byte TXC
							//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;				//					
	TxData[5] = 0x01;				//
	TxData[6] = 0x04;				//

									/*	TxData[7] = OperBuf[0];			//dennature
									TxData[8] = OperBuf[1];
									TxData[9] = OperBuf[2];
									TxData[10] = OperBuf[3];
									TxData[11] = OperBuf[4];
									TxData[12] = OperBuf[5];
									TxData[13] = OperBuf[6];		//Anneal数据
									TxData[14] = OperBuf[7];
									TxData[15] = OperBuf[8];
									TxData[16] = OperBuf[9];
									TxData[17] = OperBuf[10];
									TxData[18] = OperBuf[11];
									TxData[19] = OperBuf[12];		//Inter extension数据
									TxData[20] = OperBuf[13];
									TxData[21] = OperBuf[14];
									TxData[22] = OperBuf[15];
									TxData[23] = OperBuf[16];
									TxData[24] = OperBuf[17];
									*/
	int i;

	for (i = 0; i < 40; i++) {
		TxData[7 + i] = OperBuf[i];
	}

	for (i = 1; i < 47; i++) {
		TxData[47] += TxData[i];
	}

	if (TxData[47] == 0x17) {
		TxData[47] = 0x18;
	}

	//	else
	//		TxData[25] = TxData[25];

	TxData[48] = 0x17;
	TxData[49] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();		//
}

void COperationDlg::SetPCRCyclTempTimeNew2Seg(float den_temp, int den_time, float den_rate, float ann_temp, int ann_time, float ann_rate)
{
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

	hData = (unsigned char *)&den_rate;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];


	hData = (unsigned char *)&ann_temp;
	OperBuf[10] = hData[0];
	OperBuf[11] = hData[1];
	OperBuf[12] = hData[2];
	OperBuf[13] = hData[3];

	OperBuf[14] = ann_time >> 8;
	OperBuf[15] = ann_time;

	hData = (unsigned char *)&ann_rate;
	OperBuf[16] = hData[0];
	OperBuf[17] = hData[1];
	OperBuf[18] = hData[2];
	OperBuf[19] = hData[3];


/*	hData = (unsigned char *)&ext_temp;
	OperBuf[20] = hData[0];
	OperBuf[21] = hData[1];
	OperBuf[22] = hData[2];
	OperBuf[23] = hData[3];

	OperBuf[24] = ext_time >> 8;
	OperBuf[25] = ext_time;

	hData = (unsigned char *)&ext_rate;
	OperBuf[26] = hData[0];
	OperBuf[27] = hData[1];
	OperBuf[28] = hData[2];
	OperBuf[29] = hData[3];
*/

	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x13;			//command  TXC
	TxData[2] = 0x18;			//data length
	TxData[3] = 0x09;			//data type, date edit first byte TXC
								//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;			//RegBuf[18];						
	TxData[5] = 0x01;			//RegBuf[20];	
	TxData[6] = 0x02;			// RegBuf[21];

	TxData[7] = OperBuf[0];		//dennature数据
	TxData[8] = OperBuf[1];
	TxData[9] = OperBuf[2];
	TxData[10] = OperBuf[3];
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	
	TxData[17] = OperBuf[10];	
	TxData[18] = OperBuf[11];
	TxData[19] = OperBuf[12];		// extension数据
	TxData[20] = OperBuf[13];
	TxData[21] = OperBuf[14];
	TxData[22] = OperBuf[15];
	TxData[23] = OperBuf[16];
	TxData[24] = OperBuf[17];
	TxData[25] = OperBuf[18];
	TxData[26] = OperBuf[19];

/*	TxData[27] = OperBuf[20];	
	TxData[28] = OperBuf[21];
	TxData[29] = OperBuf[22];		// extension数据
	TxData[30] = OperBuf[23];
	TxData[31] = OperBuf[24];
	TxData[32] = OperBuf[25];
	TxData[33] = OperBuf[26];
	TxData[34] = OperBuf[27];
	TxData[35] = OperBuf[28];
	TxData[36] = OperBuf[29];
*/
	for (int i = 1; i < 27; i++) {
		TxData[27] += TxData[i];
	}

	if (TxData[27] == 0x17)
		TxData[27] = 0x18;
	//	else
	//		TxData[28] = TxData[28];

	TxData[28] = 0x17;
	TxData[29] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//
}

void COperationDlg::SetMeltCurve(float start, float end)
{
	OperDlgFlag = OPETOGRAMSG;

//	float start = 60;
//	float end = 95;
	float rate = 1;

	unsigned char * hData;

	hData = (unsigned char *)&start;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	hData = (unsigned char *)&end;
	OperBuf[4] = hData[0];
	OperBuf[5] = hData[1];
	OperBuf[6] = hData[2];
	OperBuf[7] = hData[3];

	hData = (unsigned char *)&rate;
	OperBuf[8] = hData[0];
	OperBuf[9] = hData[1];
	OperBuf[10] = hData[2];
	OperBuf[11] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x0e;		//data length
	TxData[3] = 0x0b;		//data type

	TxData[4] = 0x01;		// start

	TxData[5] = OperBuf[0];
	TxData[6] = OperBuf[1];
	TxData[7] = OperBuf[2];
	TxData[8] = OperBuf[3];

	TxData[9] = OperBuf[4];
	TxData[10] = OperBuf[5];
	TxData[11] = OperBuf[6];
	TxData[12] = OperBuf[7];

	TxData[13] = OperBuf[8];
	TxData[14] = OperBuf[9];
	TxData[15] = OperBuf[10];
	TxData[16] = OperBuf[11];

	for (int i = 1; i < 17; i++)
		TxData[17] += TxData[i];
	if (TxData[17] == 0x17)
		TxData[17] = 0x18;
//	else
//		TxData[17] = TxData[17];

	TxData[18] = 0x17;
	TxData[19] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//
}


void COperationDlg::SetPCRCyclTempTime4Seg1303(float den_temp, int den_time, float ann_temp, int ann_time, float ann2_temp, int ann2_time, float ext_temp, int ext_time) {

	// third send
	OperDlgFlag = OPEREADSTATUS;

	unsigned char * hData = (unsigned char *)&den_temp;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	OperBuf[4] = den_time >> 8;
	OperBuf[5] = den_time;

	hData = (unsigned char *)&ann_temp;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	OperBuf[10] = ann_time >> 8;
	OperBuf[11] = ann_time;

	hData = (unsigned char *)&ann2_temp;
	OperBuf[12] = hData[0];
	OperBuf[13] = hData[1];
	OperBuf[14] = hData[2];
	OperBuf[15] = hData[3];

	OperBuf[16] = ann2_time >> 8;
	OperBuf[17] = ann2_time;

	hData = (unsigned char *)&ext_temp;
	OperBuf[18] = hData[0];
	OperBuf[19] = hData[1];
	OperBuf[20] = hData[2];
	OperBuf[21] = hData[3];

	OperBuf[22] = ext_time >> 8;
	OperBuf[23] = ext_time;


	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x1C;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte TXC
							//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;		//RegBuf[18];						
	TxData[5] = 0x01;        //RegBuf[20];	
	TxData[6] = 0x04;       // RegBuf[21];
	TxData[7] = OperBuf[0];		//dennature数据
	TxData[8] = OperBuf[1];
	TxData[9] = OperBuf[2];
	TxData[10] = OperBuf[3];
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	TxData[19] = OperBuf[12];	//ANN2
	TxData[20] = OperBuf[13];
	TxData[21] = OperBuf[14];
	TxData[22] = OperBuf[15];
	TxData[23] = OperBuf[16];
	TxData[24] = OperBuf[17];
	TxData[25] = OperBuf[18];	//Inter extension数据
	TxData[26] = OperBuf[19];
	TxData[27] = OperBuf[20];
	TxData[28] = OperBuf[21];
	TxData[29] = OperBuf[22];
	TxData[30] = OperBuf[23];

	for (int i = 1; i < 31; i++)
		TxData[31] += TxData[i];
	if (TxData[31] == 0x17)
		TxData[31] = 0x18;
	else
		TxData[31] = TxData[31];
	TxData[32] = 0x17;
	TxData[33] = 0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}




