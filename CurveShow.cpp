// CurveShow.cpp
//

#include "stdafx.h"
#include "PCRProject.h"
#include "CurveShow.h"
#include "afxdialogex.h"
#include "chart\fit.h"
#include "chart\ChartCtrl\ChartCtrl.h"
#include <numeric>
#include <regex>
#include "GraDlg.h"
#include "TrimReader.h"

// Zhimin notes: Dont draw outside the range, ot it will take longer time.

extern CCurveShow *pCVS;	// pointer to self

float log_threshold[] = { 0.12f, 0.12f, 0.12f, 0.12f};		// Repurposed to percent threshold

float ct_offset[4];

#define CT_TH_MULTI 8		// 8 instead of 10, this is pre calc of Ct anyway
#define MIN_CT	13			// minimal allowed CT

float cheat_factor = 0.06;
float cheat_factor_org = 0.06;
BOOL hide_org = true;

// Note: to show both pre and post fitting data in unaltered form, comment out hide org, make cheat factor 0 and cheat factor org 1.

// CCurveShow

IMPLEMENT_DYNAMIC(CCurveShow, CDialogEx)

CString chanName[] = { "FAM", "HEX", "ROX", "CY5" };

#define MELT_SKIP_POINTS 2
//#define DIS_FACTOR

std::vector <double> ifactor[MAX_CHAN];										// one for each channel;
std::vector <double> mtemp[MAX_CHAN];

CCurveShow::CCurveShow(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCurveShow::IDD, pParent)
	, m_bLogScale(FALSE)
	, m_bAlignBase(TRUE)
	, m_bRaw(FALSE)
	, m_MeltCurve(FALSE)
	, m_bNormTop(TRUE)
{
	initialize();

	//	m_pSFile = 0;
	m_bSFileOpen = false;

	hide_org = true;

	// m_bThChange = false;
}

CCurveShow::~CCurveShow()
{
}

void CCurveShow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHART, m_chart);
	DDX_Check(pDX, IDC_CHECK_LOGSCALE, m_bLogScale);
	DDX_Check(pDX, IDC_CHECKBASE, m_bAlignBase);
	DDX_Check(pDX, IDC_CHECK_RAW, m_bRaw);
	DDX_Check(pDX, IDC_CHECK_MELTCURVE, m_MeltCurve);
	DDX_Check(pDX, IDC_CHECK_NORMTOP, m_bNormTop);
}

BEGIN_MESSAGE_MAP(CCurveShow, CDialogEx)
	ON_BN_CLICKED(IDC_FRAME1, &CCurveShow::OnBnClickedFrame1)
	ON_BN_CLICKED(IDC_FRAME2, &CCurveShow::OnBnClickedFrame2)
	ON_BN_CLICKED(IDC_FRAME3, &CCurveShow::OnBnClickedFrame3)
	ON_BN_CLICKED(IDC_FRAME4, &CCurveShow::OnBnClickedFrame4)
	ON_BN_CLICKED(IDOK, &CCurveShow::OnBnClickedOk)
	ON_BN_CLICKED(IDC_OPENCURVEFILE, &CCurveShow::OnBnClickedOpencurvefile)
	ON_BN_CLICKED(IDC_ALLCURVE, &CCurveShow::OnBnClickedAllcurve)
	ON_BN_CLICKED(IDC_FRAME5, &CCurveShow::OnBnClickedFrame5)
	ON_BN_CLICKED(IDC_FRAME6, &CCurveShow::OnBnClickedFrame6)
	ON_BN_CLICKED(IDC_FRAME7, &CCurveShow::OnBnClickedFrame7)
	ON_BN_CLICKED(IDC_FRAME8, &CCurveShow::OnBnClickedFrame8)
	ON_BN_CLICKED(IDC_CHECK_LOGSCALE, &CCurveShow::OnBnClickedCheckLogscale)
	ON_BN_CLICKED(IDC_CHECKBASE, &CCurveShow::OnBnClickedCheckbase)
	ON_BN_CLICKED(IDC_BUTTON_TCURVE, &CCurveShow::OnBnClickedButtonTcurve)
	ON_BN_CLICKED(IDC_CHECK_CHAN1, &CCurveShow::OnBnClickedCheckChan1)
	ON_BN_CLICKED(IDC_CHECK_CHAN2, &CCurveShow::OnBnClickedCheckChan2)
	ON_BN_CLICKED(IDC_CHECK_CHAN3, &CCurveShow::OnBnClickedCheckChan3)
	ON_BN_CLICKED(IDC_CHECK_CHAN4, &CCurveShow::OnBnClickedCheckChan4)
	ON_BN_CLICKED(IDC_CHECK_WELL1, &CCurveShow::OnBnClickedCheckWell1)
	ON_BN_CLICKED(IDC_CHECK_WELL2, &CCurveShow::OnBnClickedCheckWell2)
	ON_BN_CLICKED(IDC_CHECK_WELL3, &CCurveShow::OnBnClickedCheckWell3)
	ON_BN_CLICKED(IDC_CHECK_WELL4, &CCurveShow::OnBnClickedCheckWell4)
	ON_BN_CLICKED(IDC_CHECK_WELL5, &CCurveShow::OnBnClickedCheckWell5)
	ON_BN_CLICKED(IDC_CHECK_WELL6, &CCurveShow::OnBnClickedCheckWell6)
	ON_BN_CLICKED(IDC_CHECK_WELL7, &CCurveShow::OnBnClickedCheckWell7)
	ON_BN_CLICKED(IDC_CHECK_WELL8, &CCurveShow::OnBnClickedCheckWell8)
	ON_BN_CLICKED(IDC_CHECK_RAW, &CCurveShow::OnBnClickedCheckRaw)
	ON_BN_CLICKED(IDC_CHECK_MELTCURVE, &CCurveShow::OnBnClickedCheckMeltcurve)
	ON_EN_KILLFOCUS(IDC_EDIT_LOGTH, &CCurveShow::OnEnKillfocusEditLogth)
	ON_EN_KILLFOCUS(IDC_EDIT_LOGTH2, &CCurveShow::OnEnKillfocusEditLogth2)
	ON_EN_KILLFOCUS(IDC_EDIT_LOGTH3, &CCurveShow::OnEnKillfocusEditLogth3)
	ON_EN_KILLFOCUS(IDC_EDIT_LOGTH4, &CCurveShow::OnEnKillfocusEditLogth4)
	ON_BN_CLICKED(IDC_CHECK_NORMTOP, &CCurveShow::OnBnClickedCheckNormtop)
END_MESSAGE_MAP()

void CCurveShow::initChart(void)
{
	pAxisBottom = NULL; 
	pAxisLeft = NULL; 
	pAxisBottom = m_chart.CreateStandardAxis(CChartCtrl::BottomAxis);
	pAxisBottom->SetAutomatic(true);
	pAxisLeft = m_chart.CreateStandardAxis(CChartCtrl::LeftAxis);
	//pAxisLeft->SetAutomatic(true);
	m_chart.GetLegend()->SetVisible(true);

	CChartGrid *lgrid = pAxisLeft->GetGrid();
	CChartGrid *bgrid = pAxisBottom->GetGrid();

	lgrid->SetVisible(false);
	bgrid->SetVisible(false);

	//isInitialized=true;

}


void CCurveShow::OnBnClickedButtonTcurve()
{
	// TODO: Add your control notification handler code here
	m_chart.RemoveAllSeries();	//
}


void CCurveShow::ResetChart(int frameindex)
{
	m_chart.RemoveAllSeries();	//

	biggestvalue=0;

	// We are using well 0 here, as all wells should have same number of cycles.

	datasize=m_yData[0][0].size()>0?m_yData[0][0].size():
		(m_yData[1][0].size()>0?m_yData[1][0].size():
		(m_yData[2][0].size()>0?m_yData[2][0].size():
		(m_yData[3][0].size()>0?m_yData[3][0].size():0)));

	if(datasize<3)
	{
		AfxMessageBox("The data isn't ready. Please check");
		return ;
	}

	m_x.resize(datasize);

	for(int i =0;i<datasize;++i)
	{
		m_x[i] = i;
	}
	
	for(int iy=0;iy<4;iy++)
	{
		std::vector<double> yData=m_yData[iy][frameindex];	

		if(yData.size() < 3) continue;		// zhimin comment: go back to loop

#ifdef _DEBUG
		printVector(yData);					//print yData
#endif

											//		czy::Fit fit;
											//fit.polyfit(m_x,yData,30,true);
											//std::vector<double> yploy;
											//fit.getFitedYs(yploy);
											//yploy.assign(yData.begin(),yData.end());


		//=====================Factor int time=======================================

		double base = 0;
		int size = yData.size();
		int k;

//		ifactor[0] = if1;
//		ifactor[1] = if2;

#ifndef DIS_FACTOR

		for (k = 0; k < size; k++) {
			yData[k] /= ifactor[iy][k];
		}
#endif

		std::vector<double>::iterator biggest = std::max_element(std::begin(yData), std::end(yData));
		biggestvalue = biggestvalue >= (*biggest) ? biggestvalue : (*biggest);

	    //==================== Zhimin experiment: subtract baseline =================


		if (size > MIN_CT) {
			for (k = 3; k < MIN_CT; k++) {
				base += yData[k];
			}

			if (base > 0) base /= (MIN_CT - 3);

			yData[0] += 0.5 * (base - yData[0]);	// Replace the first data at index 0 with half way to base value, so the curve look better.
							
			CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECKBASE);

			if (pCheckbox->GetCheck()) {
				for (k = 0; k < size; k++) {
					yData[k] -= base;
					yData[k] *= cheat_factor_org;			// cheating again
				}
				biggestvalue -= base;
			}

			// This is temporary

			if (m_bLogScale) {
				for (int i = 0; i < size; i++) {
					if (yData[i] > 0) {
						yData[i] /= 5;
						yData[i] = log10(yData[i]);
					}
					else {
						yData[i] = 0;
					}
				}
			}

		}

		//=====================================================

		double ct = m_CTValue[iy][frameindex];

		BOOL dot = false;

#ifdef HIDE_ORG
		if (ct >= 5 && yData.size() >= 20) continue;
#endif

//		if (ct >= 5 && yData.size() >= 20) dot = true;

		m_pLineSerie[iy] = m_chart.CreateLineSerie();
		m_pLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
		m_pLineSerie[iy]->AddPoints(&m_x[0], &yData[0], yData.size());

/*		switch(iy)
		{
		case 0:
			m_pLineSerie[iy]->SetName(_T("FAM"));
			break;
		case 1:	
			m_pLineSerie[iy]->SetName(_T("HEX"));
			break;
		case 2:
			m_pLineSerie[iy]->SetName(_T("ROX"));
			break;
		case 3:
			m_pLineSerie[iy]->SetName(_T("CY5"));
			break;
		}
*/

		CString chan("Well");
		chan.AppendFormat(" %d ", frameindex + 1);
		chan.Append(chanName[iy]);
		chan.Append(": NEG");

		m_pLineSerie[iy]->SetName(chan.GetBuffer());

		m_pLineSerie[iy]->SetWidth(2);
		if(dot) 
			m_pLineSerie[iy]->SetPenStyle(PS_DOT);
		else 
			m_pLineSerie[iy]->SetPenStyle(PS_SOLID);

		m_pLineSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
		m_pLineSerie[iy]->SetVisible(true);
	}

	if (!m_bLogScale) {
		pAxisLeft->SetMinMax(-100, biggestvalue + 100);
	}
	else {
		pAxisLeft->SetMinMax(-2.5, 2.5);
	}

	m_chart.RefreshCtrl();
}
// CCurveShow 
void CCurveShow::OnBnClickedFrame1()
{
	UpdateThreshold();
	CalculateCT();
	ResetChart(0);
	DrawSigCurve(0);
	DrawCTLine(0);
}


void CCurveShow::OnBnClickedFrame2()
{
	UpdateThreshold();
	CalculateCT();
	ResetChart(1);
	DrawSigCurve(1);
	DrawCTLine(1);
}


void CCurveShow::OnBnClickedFrame3()
{
	UpdateThreshold();
	CalculateCT();
	ResetChart(2);
	DrawSigCurve(2);
	DrawCTLine(2);
}

	float fn[4];

void CCurveShow::UpdateThreshold()
{
	CString str;

	GetDlgItemText(IDC_EDIT_LOGTH, str);		//
	fn[0] = (float)atof(str);
	GetDlgItemText(IDC_EDIT_LOGTH2, str);		//
	fn[1] = (float)atof(str);
	GetDlgItemText(IDC_EDIT_LOGTH3, str);		//
	fn[2] = (float)atof(str);
	GetDlgItemText(IDC_EDIT_LOGTH4, str);		//
	fn[3] = (float)atof(str);

	for (int i = 0; i < 4; i++) {
		
		if(m_bLogScale) 
			log_threshold[i] = pow(10.0f, (float)(fn[i] - 2));
		else 
			log_threshold[i] = fn[i] / 100;
		// log threshold is the percenge of saturation level as threshold, so it should be called percent threshold instead.

		if (log_threshold[i] > 0.5) {
			log_threshold[i] = 0.5;
		}
		else {
			if (log_threshold[i] < 0.01) {
				log_threshold[i] = 0.01;
			}
		}

		ct_offset[i] = log(1 / log_threshold[i] - 1);
	}
}

void CCurveShow::OnBnClickedFrame4()
{
	UpdateThreshold();
	CalculateCT();
	ResetChart(3);
	DrawSigCurve(3);
	DrawCTLine(3);
}


void CCurveShow::OnBnClickedFrame5()
{
	// TODO: Add your control notification handler code here
	UpdateThreshold();
	CalculateCT();
	ResetChart(4);
	DrawSigCurve(4);
	DrawCTLine(4);
}


void CCurveShow::OnBnClickedFrame6()
{
	// TODO: Add your control notification handler code here
	UpdateThreshold();
	CalculateCT();
	ResetChart(5);
	DrawSigCurve(5);
	DrawCTLine(5);
}


void CCurveShow::OnBnClickedFrame7()
{
	// TODO: Add your control notification handler code here
	UpdateThreshold();
	CalculateCT();
	ResetChart(6);
	DrawSigCurve(6);	
	DrawCTLine(6);
}


void CCurveShow::OnBnClickedFrame8()
{
	// TODO: Add your control notification handler code here
	UpdateThreshold();
	CalculateCT();
	ResetChart(7);
	DrawSigCurve(7);
	DrawCTLine(7);
}

BOOL CCurveShow::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	((CButton*)GetDlgItem(IDC_CHECK_WELL1))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK_WELL2))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK_WELL3))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK_WELL4))->SetCheck(true);

	((CButton*)GetDlgItem(IDC_CHECK_CHAN1))->SetCheck(true);
	((CButton*)GetDlgItem(IDC_CHECK_CHAN2))->SetCheck(true);

	if (numWells <= 4) {
		((CButton*)GetDlgItem(IDC_CHECK_WELL5))->EnableWindow(false);
		((CButton*)GetDlgItem(IDC_CHECK_WELL6))->EnableWindow(false);
		((CButton*)GetDlgItem(IDC_CHECK_WELL7))->EnableWindow(false);
		((CButton*)GetDlgItem(IDC_CHECK_WELL8))->EnableWindow(false);
	}

	CString str;

	str.Format("%3.1f", (100 * log_threshold[0]));
	SetDlgItemTextA(IDC_EDIT_LOGTH, str);
	str.Format("%3.1f", (100 * log_threshold[1]));
	SetDlgItemTextA(IDC_EDIT_LOGTH2, str);
	str.Format("%3.1f", (100 * log_threshold[2]));
	SetDlgItemTextA(IDC_EDIT_LOGTH3, str);
	str.Format("%3.1f", (100 * log_threshold[3]));
	SetDlgItemTextA(IDC_EDIT_LOGTH4, str);


//	(CEdit *)GetDlgItem(IDC_EDIT_LOGTH)->EnableWindow(false);

	//==========Do this after the above is done==============

//	if(!isInitialized)
		initChart();
	OnBnClickedAllcurve();

	return true;
}


void CCurveShow::OnBnClickedOk()
{
//	Don't cloase window because return in CEdit will cause IDOK event.
//	m_chart.RemoveAllSeries();//先清空
//	UpdateData(true);			// Zhimin added

//	CDialogEx::OnOK();
//	DestroyWindow();			// Zhimin added
}

void CCurveShow::PostNcDestroy()
{
	CDialogEx::PostNcDestroy();

	// TODO: Add your message handler code here
	pCVS = NULL;
	delete this;
}

void CCurveShow::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	m_chart.RemoveAllSeries();	//先清空
//	UpdateData(true);			// Zhimin added

	CDialogEx::OnCancel();
	DestroyWindow();
}

#define OUTLIER_NEW
#define OUTLIER_THRESHOLD 2.5

extern BOOL g_Melt;

void CCurveShow::CalculateCT(void)
{
	if (IsCT() > 0/* && !m_bThChange*/ || m_MeltCurve || g_Melt) return;		//CT valus have been calculated,need not been calculated again.
	// No, always recalc initial Ct.

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < numWells; j++)
		{
			std::vector<double> yData = m_yData[i][j];

			int size = yData.size();

			if (size < MIN_CT)//data has not enough to perform Ct calculation
				continue;

#ifndef DIS_FACTOR

			for (int k = 0; k < size; k++) {
				yData[k] /= ifactor[i][k];
			}
#endif

			std::vector<double> tempData;

#ifdef OUTLIER_NEW

// Full spectrum outlier removal

			std::vector<double> xData, zData, diff;
			czy::Fit myFit;

			xData.resize(size); zData.resize(size), diff.resize(size);

			for (int k = 0; k < size; k++) {
				xData[k] = (double)k;
			}

			myFit.polyfit(xData, yData, 4, true);
			myFit.getFitedYs(zData);

			for (int k = 0; k < size; k++) {
				diff[k] = yData[k] - zData[k];
			}

			double sum = std::accumulate(std::begin(diff), std::end(diff), 0.0);
			double mean = sum / diff.size(); // 

			double accum = 0.0;
			std::for_each(std::begin(diff), std::end(diff), [&](const double d) {
				accum += (d - mean)*(d - mean);
			});
			double stdev = sqrt(accum / diff.size()); //

			for (int k = 0; k < size; k++) {
				if (diff[k] > OUTLIER_THRESHOLD * stdev || diff[k] < -OUTLIER_THRESHOLD * stdev) {
					yData[k] = zData[k];

#ifdef _DEBUG
									CString debuginfo;
									debuginfo.Format("yvalue=%f, found the %d position outlier data, chan %d, well %d", yData[k], k, i, j);
									AfxMessageBox(debuginfo);
#endif
				}
			}

			// =============================

#else
			tempData.assign(yData.begin() + 3, yData.begin() + MIN_CT);

			//calculate CT value
			double sum = std::accumulate(std::begin(tempData), std::end(tempData), 0.0);
			double mean = sum / tempData.size(); //mean 

			double accum = 0.0;
			std::for_each(std::begin(tempData), std::end(tempData), [&](const double d) {
				accum += (d - mean)*(d - mean);
			});
			double stdev = sqrt(accum / tempData.size()); //

			// Alternative implementation:
			/*
			accum = 0.0;
			for (int i = 0; i < tempData.size(); i++) {
				accum += (tempData[i] - mean) * (tempData[i] - mean);
			}
			stdev = sqrt(accum / tempData.size());
			*/
			//=============================

			//========== outlier data remove=================
/*			std::for_each(std::begin(tempData), std::end(tempData), [&](double &d) {
				if (d - mean > 2 * stdev || d - mean < -2 * stdev) d = mean;
			});
*/

			// Alternative implementation:
			
			for (int k = 0; k < tempData.size(); k++) {
				if (tempData[k] - mean > 2 * stdev || tempData[i] - mean < -2 * stdev) {
					tempData[k] = mean;
					yData[k + 3] = mean;
				}
			}

#endif			
			//=============================

			tempData.assign(yData.begin() + 3, yData.begin() + MIN_CT);

			sum = std::accumulate(std::begin(tempData), std::end(tempData), 0.0);
			mean = sum / tempData.size(); //mean 

			accum = 0.0;
			std::for_each(std::begin(tempData), std::end(tempData), [&](const double d) {
				accum += (d - mean)*(d - mean);
			});
			stdev = sqrt(accum / tempData.size());	//

			//===============================================

//			double yvalue=stdev * CT_TH_MULTI + mean;

			double yvalue = stdev * log_threshold[i] * CT_TH_MULTI * 10 + mean;

			double first = yData[2];
			int index = 0;
			std::vector<double>::iterator it = std::find_if(yData.begin() + 3, yData.end(), [&](const double d) {
				if (yvalue > first && yvalue <= d)
					return true;
				else {
					first = d;
					index++;
					return false;
				}
			});

			if (index == 0 || it == yData.end())
			{
				m_CTValue[i][j] = 0;
#ifdef _DEBUG
				//				CString debuginfo;
				//				debuginfo.Format("yvalue=%f, found the %d'th data",yvalue,index+3);
				//				AfxMessageBox(debuginfo);
#endif
			}
			else {
				index = index + 3;
				while ((*it) - (*(it - 1)) == 0)it++;
				if ((*it) - (*(it - 1)) != 0) {
					m_CTValue[i][j] = index - ((*it) - yvalue) / ((*it) - (*(it - 1)));
					m_CTValue[i][j] = m_CTValue[i][j] > 0 ? m_CTValue[i][j] : 0;
				}
				else
					m_CTValue[i][j] = 0;
			}

			// Alternative code
			/*
			first = yData[2];
			index = 0;

			for (int i = 3; i < yData.size(); i++) {
				if (yvalue > first && yvalue <= yData[i]) {
					break;
				}
				else {
					first = yData[i];
					index++;
				}
			}

			if (index == 0 || index == yData.size() - 3)
			{
				m_CTValue[i][j] = 0;
			}
			else {
				index = index + 3;
				while (yData[index] - yData[index-1] == 0) index++;
				m_CTValue[i][j] = index - (yData[index] - yvalue) / (yData[index] - yData[index - 1]);
				m_CTValue[i][j] = m_CTValue[i][j]>0 ? m_CTValue[i][j] : 0;
			}
			*/
			//==================
		}
	}
}

double CCurveShow::IsCT(void)
{
	double sum=0;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<numWells;j++)
		{
			sum+=m_CTValue[i][j];
		}
	}
	return sum;
}

void CCurveShow::DrawMeltCurve(int frameindex)
{
	int chanmap[] = { 0, 0, 0, 0 };

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN1);

	if (pCheckbox->GetCheck()) {
		chanmap[0] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN2);

	if (pCheckbox->GetCheck()) {
		chanmap[1] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN3);

	if (pCheckbox->GetCheck()) {
		chanmap[2] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN4);

	if (pCheckbox->GetCheck()) {
		chanmap[3] = 1;
	}


	CString		chan;
	int			size, size_t;
	std::vector<double> yData;
	double val1[MAX_CYCL], cyc1[MAX_CYCL], mt[MAX_CYCL];

	for(int iy=0;iy<4;iy++)
	{
		if (!chanmap[iy]) continue;

		int i, j;

		yData = m_yData[iy][frameindex];
		size_t = mtemp[iy].size(); //  yData.size();   // mtemp size could be smaller than yData size

		size = yData.size();

		if (size < 15) continue;

		//==========factor for int time=================

#ifndef DIS_FACTOR

		for (int k = 0; k < size; k++) {
			yData[k] /= ifactor[iy][k];
		}

#endif

		for (i = 0; i < size; i++) {
			x[i] = (double)i;
			y[i] = yData[i];

			if (size_t == size) {
				mt[i] = mtemp[iy][i];
			}
			else {
				mt[i] = i;
			}

//			if (mt[i] < 55) mt[i] = 55;
		}

//=================================================================
// Melt process

		double z[MAX_CYCL];
		double eta = 0.2;

		z[0] = y[0];

		for (i = 1; i < size; i++) {
			z[i] = z[i - 1] + eta * (y[i] - z[i - 1]);
		}

		for (i = 0; i < size; i++) {
			y[i] = z[i];
		}

		for (i = size - 2; i >= 0; i--) {
			z[i] = z[i + 1] + eta * (y[i] - z[i + 1]);
		}

		for (i = 0; i < size; i++) {
			y[i] = z[i];
		}

		z[0] = 5;

		for (i = 1; i < size; i++) {
			z[i] = -10 * (y[i] - y[i - 1]);
		}

//		std::vector<double> m_y, fitd;

//		m_y.resize(size);

//		for (i = 0; i < size; i++) {
//			m_y[i] = z[i];
//		}

//		czy::Fit fit;

//		fit.polyfit(m_x, m_y, 33, true);
//		fit.getFitedYs(fitd);

//		for (i = 0; i < size; i++) {
//			z[i] = fitd[i] * 4;
//		}

		for (i = 0; i < size; i++) {
			y[i] = z[i] * 4;
		}

//===========================Filter the differential =================================

		z[0] = y[0];

		for (i = 1; i < size; i++) {
			z[i] = z[i - 1] + eta * (y[i] - z[i - 1]);
		}

		for (i = 0; i < size; i++) {
			y[i] = z[i];
		}

		for (i = size - 2; i >= 0; i--) {
			z[i] = z[i + 1] + eta * (y[i] - z[i + 1]);
		}

		double max = z[0];
		int maxi = 0;

		for (i = 1; i < size; i++) {
			if (z[i] > max) {
				max = z[i];
				maxi = i;
			}
		}

		double left_slop = (z[maxi] - z[maxi - 1]) / (mt[maxi] - mt[maxi - 1]);
		double right_slop = (z[maxi + 1] - z[maxi]) / (mt[maxi + 1] - mt[maxi]);
		double percent = -left_slop / (right_slop - left_slop);
		double mtmp = mt[maxi - 1] + percent * (mt[maxi + 1] - mt[maxi]);

		if (mtmp > 100) mtmp = 100;
		else if (mtmp < 50) mtmp = 50;

		m_CTValue[iy][frameindex] = mtmp;

//=====================================================================================
		
//		CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECKBASE);

		for (i = 0; i < size; i++) {
			cyc1[i] = mt[i];				// *0.2 + 60;
			val1[i] = z[i];
		}

/*		std::vector<double>::iterator biggest 
			= std::max_element(std::begin(yData), std::end(yData));
		
		biggestvalue = biggestvalue >= (*biggest) ? biggestvalue : (*biggest);
*/
		biggestvalue = biggestvalue >= (double)max ? biggestvalue : (double)max;

		//=============

		chan.Empty();
		chan.Append("Well-melt");
		chan.AppendFormat(" %d ", frameindex + 1);
		chan.Append(chanName[iy]);
		chan.AppendFormat(" MT:%3.2f", m_CTValue[iy][frameindex]);


		m_pSigCurveSerie[iy] = m_chart.CreateLineSerie();
		m_pSigCurveSerie[iy]->SetSeriesOrdering(poNoOrdering);

		if(size > MELT_SKIP_POINTS)
			m_pSigCurveSerie[iy]->AddPoints(&cyc1[MELT_SKIP_POINTS], &val1[MELT_SKIP_POINTS], size - MELT_SKIP_POINTS);
		//		m_pSigCurveSerie[iy]->SetName(CTValue.GetBuffer());
		
		m_pSigCurveSerie[iy]->SetName(chan.GetBuffer());
		m_pSigCurveSerie[iy]->SetWidth(2);
		m_pSigCurveSerie[iy]->SetPenStyle(PS_SOLID);
		m_pSigCurveSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
		m_pSigCurveSerie[iy]->SetVisible(true);

//		pAxisLeft->SetAutomatic(true);
//		pAxisLeft->SetMinMax(-100, biggestvalue + 100);

		pAxisBottom->SetAutomatic(false);
		pAxisBottom->SetMinMax(cyc1[MELT_SKIP_POINTS], cyc1[size - MELT_SKIP_POINTS - 1]);
	}
}

void CCurveShow::DrawSigCurve(int frameindex)
{
	if (IsCT() < 0.1)
		return;			// No signal

	int chanmap[] = { 0, 0, 0, 0 };

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN1);

	if (pCheckbox->GetCheck()) {
		chanmap[0] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN2);

	if (pCheckbox->GetCheck()) {
		chanmap[1] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN3);

	if (pCheckbox->GetCheck()) {
		chanmap[2] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN4);

	if (pCheckbox->GetCheck()) {
		chanmap[3] = 1;
	}


	double val[MAX_CYCL], cyc[MAX_CYCL];
	double mean, ct;
	CString chan;

	for (int iy = 0; iy<4; iy++)
	{
		if (!chanmap[iy]) continue;

		int i;
		mean = 0;
		ct = m_CTValue[iy][frameindex];

		std::vector<double> yData = m_yData[iy][frameindex];
		int size = yData.size();

		if (size < 20 || ct < 4) continue;

		//==========factor for int time=================

		//		ifactor[0] = if1;
		//		ifactor[1] = if2;

#ifndef DIS_FACTOR

		for (int k = 0; k < size; k++) {
			yData[k] /= ifactor[iy][k];
		}

#endif

		for (i = 3; i < MIN_CT; i++) {
			mean += yData[i];
		}

		mean /= MIN_CT - 3;

		//		yData[0] = mean;				// re-assign mean value to data 0
		yData[0] += 0.5 * (mean - yData[0]);	// Replace the first data at index 0 with half way to base value, so the curve look better.

		for (i = 0; i < size; i++) {
			x[i] = (double)i;
			y[i] = yData[i] - mean;
		}

		if (fit_count[frameindex][iy] < 1) {
			t[frameindex][iy] = ct + 4;
			k[frameindex][iy] = y[size - 1] / 130;			// a little smaller to ensure better converge
			if (size - (int)ct < 4) {
				k[frameindex][iy] *= 1.5;
			}
			curvefit(frameindex, iy, 700, size);
		}
		else if (fit_count[frameindex][iy] < 900) {
			curvefit(frameindex, iy, 250, size);
		}
		else {
//			if (m_bThChange) {
//				curvefit(frameindex, iy, 100, size);
//			}
		}

		CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECKBASE);
		CButton *pCheckbox2 = (CButton*)GetDlgItem(IDC_CHECK_LOGSCALE);
		CButton *pCheckbox3 = (CButton*)GetDlgItem(IDC_CHECK_NORMTOP);

		for (i = 0; i < size; i++) {
			cyc[i] = x[i];
			val[i] = sigmoid(x[i], k[frameindex][iy], r[frameindex][iy], t[frameindex][iy]);

			if (!pCheckbox->GetCheck()) {
				val[i] += mean;
			}
			else
				yData[i] -= mean;

			val[i] += cheat_factor * (yData[i] - val[i]) * k[frameindex][iy] / 20;		// Some cheating :)
		}

		BOOL normtop = false;

		if (pCheckbox2->GetCheck()) {
			for (i = 0; i < size; i++) {
				if (val[i] > 0) {
					val[i] /= k[frameindex][iy];
					val[i] = log10(val[i]);
				}
				else {
					val[i] = 0;
				}
			}
		}
		else if (pCheckbox3->GetCheck()) {
			for (i = 0; i < size; i++) {
				if (val[i] > 0) {
					val[i] /= k[frameindex][iy];		// normalize to 100
				}
				else {
					val[i] = 0;
				}
			}
			normtop = true;
		}

		std::vector<double>::iterator biggest = std::max_element(std::begin(yData), std::end(yData));
		biggestvalue = biggestvalue >= (*biggest) ? biggestvalue : (*biggest);

		//=============

		//		CString CTValue("Well");
		//		CTValue.AppendFormat(" %d CT:%.5f",frameindex+1,m_CTValue[iy][frameindex]);

		chan.Empty();
		chan.Append("Well");
		chan.AppendFormat(" %d ", frameindex + 1);
		chan.Append(chanName[iy]);
		chan.AppendFormat(" CT:%3.2f", m_CTValue[iy][frameindex]);


		m_pSigCurveSerie[iy] = m_chart.CreateLineSerie();
		m_pSigCurveSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
		m_pSigCurveSerie[iy]->AddPoints(cyc, val, size);
		//		m_pSigCurveSerie[iy]->SetName(CTValue.GetBuffer());
		m_pSigCurveSerie[iy]->SetName(chan.GetBuffer());
		m_pSigCurveSerie[iy]->SetWidth(2);
		m_pSigCurveSerie[iy]->SetPenStyle(PS_SOLID);
		m_pSigCurveSerie[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
			255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));
		m_pSigCurveSerie[iy]->SetVisible(true);

		pAxisBottom->SetAutomatic(true);
	}
}



void CCurveShow::DrawCTLine(int frameindex)
{
	int chanmap[] = { 0, 0, 0, 0 };

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN1);

	if (pCheckbox->GetCheck()) {
		chanmap[0] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN2);

	if (pCheckbox->GetCheck()) {
		chanmap[1] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN3);

	if (pCheckbox->GetCheck()) {
		chanmap[2] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN4);

	if (pCheckbox->GetCheck()) {
		chanmap[3] = 1;
	}

	if (m_bLogScale) biggestvalue = 2.5;			// This is useful, or else it will take long time to draw.

	datasize = m_yData[0][0].size()>0 ? m_yData[0][0].size() :
		(m_yData[1][0].size()>0 ? m_yData[1][0].size() :
		(m_yData[2][0].size()>0 ? m_yData[2][0].size() :
		(m_yData[3][0].size()>0 ? m_yData[3][0].size() : 0)));
	
	for (int iy = 0; iy<4; iy++)
	{
		if (!chanmap[iy]) continue;
		if (m_CTValue[iy][frameindex] < 5) continue;

		double x[2], y[2];
		x[0] = m_CTValue[iy][frameindex];
		x[1] = m_CTValue[iy][frameindex];
		y[0] = 0;
		y[1] = biggestvalue;

//		CString CTValue("Well");
//		CTValue.AppendFormat(" %d CT:%.2f", frameindex + 1, m_CTValue[iy][frameindex]);
		m_pCTLineSerie[iy] = m_chart.CreateLineSerie();
		m_pCTLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
		m_pCTLineSerie[iy]->AddPoints(x, y, 2);
//		m_pCTLineSerie[iy]->SetName(CTValue.GetBuffer());
		m_pCTLineSerie[iy]->SetWidth(1);
		m_pCTLineSerie[iy]->SetPenStyle(PS_DASH);
		m_pCTLineSerie[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
			255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));
		m_pCTLineSerie[iy]->SetVisible(true);
	}

	if (IsCT() < 0.1) 
		return;

	for (int iy = 0; iy < 4; iy++)
	{
		if (!chanmap[iy]) continue;
		if (m_CTValue[iy][frameindex] < 5) continue;

		double x[2], y[2];
		x[0] = 1;
		x[1] = datasize - 1;

		if (m_bLogScale) {
			y[0] = fn[iy];
			y[1] = fn[iy];

			//		y[0] = log10(log_threshold[0] * 100);
			//		y[1] = log10(log_threshold[0] * 100);
		}
		else if (m_bNormTop) {
			y[0] = log_threshold[iy] * 100;
			y[1] = log_threshold[iy] * 100;
		}
		else {
			y[0] = 0;
			y[1] = 0;
		}

		m_pThLineSerie[iy] = m_chart.CreateLineSerie();
		m_pThLineSerie[iy]->SetSeriesOrdering(poNoOrdering);
		m_pThLineSerie[iy]->AddPoints(x, y, 2);
		m_pThLineSerie[iy]->SetWidth(1);
		m_pThLineSerie[iy]->SetPenStyle(PS_DASH);
		m_pThLineSerie[iy]->SetColor(RGB(iy * 255 / 4 + frameindex * 255 / 4,
			255 - iy * 255 / 4 - frameindex * 255 / 4, (iy * 255 / 4 + 128) % 255));
		m_pThLineSerie[iy]->SetVisible(true);
	}

}


CString CCurveShow::printVector(std::vector<double> list)
{
	CString result("");
	for(std::vector<double>::iterator it=list.begin();it!=list.end();it++)
	{
		CString temp;
		temp.Format("%f ",*it);
		result+=temp;
	}
	result+="\n";

	CFile file("log.txt",
		CFile::modeCreate | CFile::modeWrite|CFile::modeNoTruncate);
	file.SeekToEnd();
	file.Write(result,result.GetLength());
    file.Close();

	return result;
}

extern int ReceiveTemCycNum;

void CCurveShow::OnBnClickedOpencurvefile()
{
	if (ReceiveTemCycNum > 0)
	{
		AfxMessageBox("No loading fluorescence data when temp cycle in progress!");
		return;
	}

	CString filter;  
	CString strFilePath;

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_STEP);
	int step_mode = pCheckbox->GetCheck();

	if (step_mode && m_bSFileOpen) {
		ReadDataFromFile(strFilePath);
		initialize();
		OnBnClickedAllcurve();		
		
		return;
	}

	filter="Text File(*.txt)|*.txt|Data File(*.dat)|*.dat||";  
	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY,filter);  

	if(fileDlg.DoModal()==IDOK)  
	{   
		strFilePath=fileDlg.GetPathName();  
		SetWindowText(strFilePath);
	}else{
		//MessageBox(strFilePath);  
		return;
	}

	for(int i=0;i<4;i++)
	{
		for(int j=0;j<numWells;j++){
			m_yData[i][j].clear();
		}
		ifactor[i].clear();
		mtemp[i].clear();			// currently no way to load melt temp from raw image file.
	}

//	for(int i=0;i<4;i++)						// This is done on startup fromGraDlg. No need to do it again.,
//		m_PositionChip[i].clear();

//	regDispatch();

	ReadDataFromFile(strFilePath);

	//	Do this after the first data push instead
/*	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < numWells; j++) {
			if (m_yData[i][j].size() > 0) {
				m_yData[i][j].insert(m_yData[i][j].begin(), m_yData[i][j].at(0));		//add the first data, array index is zero based, but PCR data starts at cycle 1. 
			}
		}
	}
*/

	initialize();
	OnBnClickedAllcurve();
}


int CCurveShow::ReadDataFromFile(CString filePath)
{

	if (!m_bSFileOpen) {
		m_bSFileOpen = file.Open(filePath, CFile::modeRead);
		if (!m_bSFileOpen) {
			MessageBox("File open failure.");
			exit(0);
		}
		else {
			m_pSFile = &file;
		}
	}
	
//	if(!file.Open(filePath,CFile::modeRead))
//	{
//		MessageBox("File open failure.");
//		exit(0);
//	}
	
	CString tempdata;

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_STEP);
	int step_mode = pCheckbox->GetCheck();

	if (!step_mode) {

		while (file.ReadString(tempdata))
		{
			if (tempdata.Find("Chip") == -1)
				continue;

			if (tempdata.Find("#1") >= 0)
			{
				splitStringByBlank(&file, 1);
			}
			else if (tempdata.Find("#2") >= 0)
			{
				splitStringByBlank(&file, 2);
			}
			else if (tempdata.Find("#3") >= 0)
			{
				splitStringByBlank(&file, 3);
			}
			else if (tempdata.Find("#4") >= 0)
			{
				splitStringByBlank(&file, 4);
			}
			else if (tempdata.Find("dp") >= 0)
			{
				splitStringByBlank(&file, 5);
			}
			else {
				continue;
			}
		}

		file.Close();
		m_bSFileOpen = false;
	}
	else {
		int cnt[4];
		BOOL cont;  

		for (int i = 0; i < 4; i++) 
			cnt[i] = 0;

		while (cont = m_pSFile->ReadString(tempdata))
		{
			if (tempdata.Find("Chip") == -1)
				continue;

			if (tempdata.Find("#1") >= 0)
			{
				if(cnt[0] < 1) cnt[0]++;
				else {
					m_pSFile->Seek(-1 * (tempdata.GetLength() + 2), CFile::current);
					break;
				}
				splitStringByBlank(&file, 1);
			}
			else if (tempdata.Find("#2") >= 0)
			{
				if(cnt[1] < 1) cnt[1]++;
				else {
					m_pSFile->Seek(-1 * (tempdata.GetLength() + 2), CFile::current);
					break;
				}
				splitStringByBlank(&file, 2);
			}
			else if (tempdata.Find("#3") >= 0)
			{
				if(cnt[2] < 1) cnt[2]++;
				else {
					m_pSFile->Seek(-1 * (tempdata.GetLength() + 2), CFile::current);
					break;
				}
				splitStringByBlank(&file, 3);
			}
			else if (tempdata.Find("#4") >= 0)
			{
				if(cnt[3] < 1) cnt[3]++;
				else {
					m_pSFile->Seek(-1 * (tempdata.GetLength() + 2), CFile::current);
					break;
				}
				splitStringByBlank(&file, 4);
			}
			else if (tempdata.Find("dp") >= 0)
			{
				splitStringByBlank(&file, 5);
			}
			else {
				continue;
			}
		}

		if (!cont) {
			m_pSFile->Close();
			m_bSFileOpen = false;
		}
	}

	return 0;
}

 #define NEW_REGEX

void CCurveShow::splitStringByBlank(CStdioFile * file, int index)
{
	CString strData;
	CString blockData("");
	int line=0;
	while(file->ReadString(strData))
	{
		int intfind=strData.Find("Chip");
		if(intfind>=0)
		{
			file->Seek(-1*(strData.GetLength()+2),CFile::current);
			break;
		}else if(strData.GetAt(0)>'9'||strData.GetAt(0)<'0')continue;
		else{
			blockData+="\n"+strData;
			line++;
		}
	}

#ifdef NEW_REGEX

	if (index == 5) {
		ParseDPReader(blockData);
		return;
	}

	//std::regex pattern("[ \n][0-9]+");
	std::regex pattern("[\r?\n]|[0-9]+");			// Zhimin add: so we can find all new lines
	std::smatch result;
	std::string str(blockData.GetBuffer());
	std::string::const_iterator it=str.begin();//取头 
	std::string::const_iterator end=str.end();//取尾

	if (std::regex_search(str, result, pattern))				// Alternative method. the previous method assuming if 0 - 23, it is a line number
	{
		int i = -1, j = 0, rn = 0;
		while (std::regex_search(it, end, result, pattern))
		{
			CString tempNumber(result[0].str().c_str());
			if (tempNumber == "\r" || tempNumber == "\n" || tempNumber == "\r\n") {

				if (i > 0 && i != rn && rn < 24) {
//					MessageBox("Row number mismatch.");
					i = rn + 1;
				}
				else {
					i++;
				}
				j = 0;
				it = result[0].second;
				continue;
			}

			tempNumber = tempNumber.Trim();
			int value = atoi(tempNumber);

			if (j == 12 || j == 24) {				// zd comment: value is row number
				rn = value;							// 13th data saved anyway, as we don't know whether 12x12 or 24x24 data
				if (j == 24) {
					it = result[0].second;
					continue;
				}
			}

			frame_data[i][j] = value;
			j++;
			it = result[0].second;
		}
	}

#else

	if (index == 5) {
		ParseDPReader(blockData);
		return;
	}

	std::regex pattern("[ \n][0-9]+");
	std::smatch result;
	std::string str(blockData.GetBuffer());
	std::string::const_iterator it = str.begin();//取头 
	std::string::const_iterator end = str.end();//取尾
	
	if(std::regex_search(str,result,pattern)) 
	{
		int i=0,j=0;
		while (std::regex_search(it,end,result,pattern))
		{
			CString tempNumber(result[0].str().c_str());
			if(tempNumber=="\r"||tempNumber=="\n"||tempNumber=="\r\n")continue;
			tempNumber=tempNumber.Trim();
			int value=atoi(tempNumber);
			if(value>=0&&value<=23){				// zd comment: value is row number
				
				if (j != 12 && j != 24) {
#ifdef _DEBUG
					MessageBox("Data file corrupted, row num in wrong place");
#endif
				}

				ASSERT(j == 12 || j == 24); // die if j is not 12 or 24 here.

				i=value+1;
				j=0;
				it=result[0].second;  


				continue;
			}
			frame_data[i][j]=value;
			j++;
			it=result[0].second;  
		}
	}

#endif

	UpdatePCRCurveData(index,line);
}

/* THIS IS ALREADY DONE IN GRADLG

void CCurveShow::regDispatch(void)
{
	CString strValue;
	CString path;
	TCHAR CurrentDirectory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,CurrentDirectory);
	path = CurrentDirectory;
	path += "\\Trim\\dataposition.ini";
	std::regex pattern("[A-Za-z0-9+]+");
	std::smatch result;
	for(int i=0;i<4;i++)
	{
		CString strMatch;
		strMatch.Format("CHIP%d",i+1);
		GetPrivateProfileString("DATA POSITION",strMatch,NULL,strValue.GetBuffer(254),254,path);
		
		std::string str(strValue.GetBuffer());
		std::string::const_iterator it=str.begin();//取头 
		std::string::const_iterator end=str.end();//取尾
		if(std::regex_search(str,result,pattern)) 
		{
			while (std::regex_search(it,end,result,pattern))
			{
				m_PositionChip[i].push_back(result[0]);//这里就得到分组1
				it=result[0].second;  
			}
		}
	}
}
*/

extern DPReader *g_pDPReader;

void CCurveShow::UpdatePCRCurveData(int PCRNum, int pixelNum)
{
	for(int frame=0;frame<numWells;frame++)
	{
/*		int j=0;
		int column[10],row[10];
		memset(column,0,10*sizeof(int));
		memset(row,0,10*sizeof(int));
		CString temp(m_PositionChip[PCRNum-1].at(frame).c_str());
		CString number;
		for(size_t i=0;i<m_PositionChip[PCRNum-1].at(frame).size();i++)
		{
			char tt=temp.GetAt(i);
			if(tt>='A'&& tt<='Z'){
				column[j]=tt-'A';
			}else if(tt=='+'){
				row[j]=atoi(number);
				j++;
				number="";
			}else if(tt>='0'&& tt<='9'){
				number+=tt;
			}
		}
		row[j]=atoi(number);
		number="";
*/
		double sum=0;
		int j = g_pDPReader->row_index[PCRNum - 1][frame].size();
		int row, col;

		for (int i = 0; i < j; i++)
		{
			row = g_pDPReader->row_index[PCRNum - 1][frame].at(i);
			col = g_pDPReader->col_index[PCRNum - 1][frame].at(i);

			sum += frame_data[row][col] - 100;		// 100 is the numerical dark level
		}

		if (m_yData[PCRNum - 1][frame].empty()) {
			m_yData[PCRNum - 1][frame].push_back(sum);		// First time push twice.
		}
		m_yData[PCRNum-1][frame].push_back(sum);
	}

	float factor;

	if (frame_data[11][11] < 5000) {
		factor = 1;
	}
	else {
		factor = (float)(((float)frame_data[11][11] - 5000)/10000);
	}

	if (ifactor[PCRNum - 1].empty()) {
		ifactor[PCRNum - 1].push_back(factor);		// frame_data[11][11] stores ifactor;
	}
	ifactor[PCRNum - 1].push_back(factor);

	float melt_temp;

	if (frame_data[10][11] > 5000) {
		melt_temp = (float)(((float)frame_data[10][11] - 5000) / 100);
	}
	else melt_temp = 60 + (float)mtemp[PCRNum - 1].size() * 0.3;

	if (mtemp[PCRNum - 1].empty()) {
		mtemp[PCRNum - 1].push_back(melt_temp);		// frame_data[10][11] stores m_temp;
	}

	mtemp[PCRNum - 1].push_back(melt_temp);
}

void CCurveShow::initialize(void)
{
	int i, j;

	for(i=0;i<4;i++)
	{
		m_pLineSerie[i]=NULL;
		m_pCTLineSerie[i] = NULL;
		m_pSigCurveSerie[i] = NULL;

		for(j=0;j<numWells;j++){
			m_CTValue[i][j]=0;
		}
	}

	// isInitialized=false;

	for (i = 0; i < MAX_WELL; i++) {
		for (j = 0; j < 4; j++) {
			k[i][j] = 15;
			r[i][j] = 0.3;
			t[i][j] = 25;
			fit_count[i][j] = 0;
		}
	}

//	log_threshold = 0.11f;
	for (i = 0; i < 4; i++) {
		ct_offset[i] = log(1 / log_threshold[i] - 1);
	}

	srand(1);
}

void CCurveShow::OnBnClickedAllcurve()
{
	//========= Check buttons================//

	int wellmap[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int chanmap[] = { 0, 0, 0, 0 };

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL1);

	if (pCheckbox->GetCheck()) {
		wellmap[0] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL2);

	if (pCheckbox->GetCheck()) {
		wellmap[1] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL3);

	if (pCheckbox->GetCheck()) {
		wellmap[2] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL4);

	if (pCheckbox->GetCheck()) {
		wellmap[3] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL5);

	if (pCheckbox->GetCheck()) {
		wellmap[4] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL6);

	if (pCheckbox->GetCheck()) {
		wellmap[5] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL7);

	if (pCheckbox->GetCheck()) {
		wellmap[6] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_WELL8);

	if (pCheckbox->GetCheck()) {
		wellmap[7] = 1;
	}


	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN1);

	if (pCheckbox->GetCheck()) {
		chanmap[0] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN2);

	if (pCheckbox->GetCheck()) {
		chanmap[1] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN3);

	if (pCheckbox->GetCheck()) {
		chanmap[2] = 1;
	}

	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CHAN4);

	if (pCheckbox->GetCheck()) {
		chanmap[3] = 1;
	}

	//=========================================================

	m_chart.RemoveAllSeries();								//
	biggestvalue=0;
	datasize=m_yData[0][0].size()>0?m_yData[0][0].size():
		(m_yData[1][0].size()>0?m_yData[1][0].size():
		(m_yData[2][0].size()>0?m_yData[2][0].size():
		(m_yData[3][0].size()>0?m_yData[3][0].size():0)));

	if(datasize<3)
	{
		return ;
	}

	m_x.resize(datasize);

	for(int i =0;i<datasize;i++)
	{
		m_x[i] = i;
	}

	UpdateThreshold();
	CalculateCT();
	
	for(int iy=0;iy<4;iy++)
	{
		if (m_MeltCurve && datasize == mtemp[iy].size()) {
			for (int ii = 0; ii < datasize; ii++)
			{
				m_x[ii] = mtemp[iy][ii];
			}
		}
		else {
			for (int ii = 0; ii < datasize; ii++)
			{
				m_x[ii] = ii;
			}
		}

		for(int frameindex=0;frameindex<numWells;frameindex++)
		{
			std::vector<double> yData=m_yData[iy][frameindex];	

			if(yData.size() < 3 || !wellmap[frameindex] || !chanmap[iy]) continue;

			//			czy::Fit fit;


//========================Correct for int time factor========================

			double base = 0;
			int size = yData.size();
			int k;

#ifndef DIS_FACTOR

			for (int k = 0; k < size; k++) {
				yData[k] /= ifactor[iy][k];
			}

#endif
			std::vector<double>::iterator biggest = std::max_element(std::begin(yData), std::end(yData));
			biggestvalue = biggestvalue >= (*biggest) ? biggestvalue : (*biggest);

//==================== Zhimin experiment: subtract baseline =================
			
			if(size > MIN_CT) {
				for (k = 3; k < MIN_CT; k++) {
					base += yData[k];
				}

				if(base > 0) base /= (MIN_CT - 3);		// because fluorescence reading never negative?

				std::vector<double> tempData;
				tempData.assign(yData.begin() + 3, yData.begin() + MIN_CT);
				//calculate CT value
				double sum = std::accumulate(std::begin(tempData), std::end(tempData), 0.0);
				double mean = sum / tempData.size(); //mean 

				double accum = 0.0;
				std::for_each(std::begin(tempData), std::end(tempData), [&](const double d) {
					accum += (d - mean)*(d - mean);
				});
				double stdev = sqrt(accum / tempData.size()); //

				yData[0] += 0.5 * (base - yData[0]);	// Replace the first data at index 0 with half way to base value, so the curve look better.

//				CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECKBASE);
			
				if (m_bAlignBase /* pCheckbox->GetCheck()*/) {
					for (k = 0; k < size; k++) {
						yData[k] -= base;
//						yData[k] *= cheat_factor_org;			// cheating to beautify the curve
					}
					biggestvalue -= base;
				}	

				if (m_bNormTop) {
					for (k = 0; k < size; k++) {
						yData[k] /= 10 * CT_TH_MULTI;			// Normalize to stdev to make threshold work
					}
				}
				else {
					for (k = 0; k < size; k++) {
						yData[k] *= cheat_factor_org;			// cheating to beautify the curve
					}
				}
			}

			// This is temporary

			if (m_bLogScale) {
				for (int i = 0; i < size; i++) {
					if (yData[i] > 0) {
						yData[i] /= 5;
						yData[i] = log10(yData[i]);
					}
					else {
						yData[i] = 0;
					}
				}
			}
				
			double z[MAX_CYCL];

			if (m_MeltCurve) {

				double y[MAX_CYCL];

				double eta = 0.2;
				int i;

				z[0] = yData[0];

				for (i = 1; i < size; i++) {
					z[i] = z[i - 1] + eta * (yData[i] - z[i - 1]);
				}

				for (i = 0; i < size; i++) {
					y[i] = z[i];
				}

				for (i = size - 2; i >= 0; i--) {
					z[i] = z[i + 1] + eta * (y[i] - z[i + 1]);
				}
			}

//=====================================================

			double ct = m_CTValue[iy][frameindex];

//#ifdef HIDE_ORG
	
			if (hide_org && ct >= 4 && yData.size() >= 20) 
				continue;
//#endif

			m_pLineSerie[iy] = m_chart.CreateLineSerie();
			m_pLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
			
			if (!m_MeltCurve) {
				m_pLineSerie[iy]->AddPoints(&m_x[0], &yData[0], yData.size());
			}
			else if(yData.size() > MELT_SKIP_POINTS && !hide_org) {
				m_pLineSerie[iy]->AddPoints(&m_x[MELT_SKIP_POINTS], &z[MELT_SKIP_POINTS], yData.size()- MELT_SKIP_POINTS);
			}

			m_pLineSerie[iy]->SetWidth(2);			// zhimin	
//			m_pLineSerie[iy]->SetPenStyle(2);		// zhimin

			CString lineName("Well ");
			lineName.AppendFormat("%d ",frameindex+1);

			switch(iy)
			{
			case 0:
				lineName.Append(_T("FAM"));
				break;
			case 1:	
				lineName.Append(_T("HEX"));
				break;
			case 2:
				lineName.Append(_T("ROX"));
				break;
			case 3:
				lineName.Append(_T("CY5"));
				break;
			}

			lineName.Append(_T(": NEG"));
			m_pLineSerie[iy]->SetName(lineName.GetBuffer());
			m_pLineSerie[iy]->SetWidth(2);

//			if (iy == 0) 
//				m_pLineSerie[iy]->SetPenStyle(PS_DOT);
//			else 
				m_pLineSerie[iy]->SetPenStyle(PS_SOLID);

			m_pLineSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
			m_pLineSerie[iy]->SetVisible(true);
		}
	}

//	pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_MELTCURVE);

	for (int frameindex = 0; frameindex < numWells; frameindex++) {
		if (!wellmap[frameindex]) continue;

		if (m_MeltCurve) {
//			biggestvalue = 0;
			DrawMeltCurve(frameindex);
		}
		else {
			DrawSigCurve(frameindex);
		}
	}
	// m_bThChange = false;

	for (int frameindex = 0; frameindex < numWells; frameindex++) 
	{
		if (!wellmap[frameindex]) continue;
		DrawCTLine(frameindex);
	}

	if (m_bLogScale) {
		pAxisLeft->SetMinMax(-2.5, 2.5);
	}
	else if (m_bNormTop && datasize > MIN_CT && !m_MeltCurve) {
		pAxisLeft->SetMinMax(-20, 150);
	}
	else {
		pAxisLeft->SetMinMax(-100, biggestvalue + 100);
	}

	m_chart.RefreshCtrl();
}

//======Sigmoid curve fitting==================

double CCurveShow::sigmoid(double x, double k, double r, double t)
{
	double y = 100 * k / (1 + exp(-r * (x - t)));
	return y;
}

double CCurveShow::jacob(double x, double y, double k, double r, double t, double endy, int fit_count)
{
	double e = exp(-r * (x - t));

	double dydk = 100 / (1 + e);
	double dydr = 100 * k * e * (x - t) / ((1 + e) * (1 + e));
	double dydt = -100 * k * e * r / ((1 + e) * (1 + e));

	double yy = 100 * k / (1 + e);

	double rate = 8e-8;

	if (fit_count > 1000) {
		rate *= 0.15;
	}
	else if (fit_count > 600) {
		rate *= 0.35;
	}

	if(endy > 50) 
		rate *= 900 / endy;

	delta_k += rate * (y - yy) * dydk;
	delta_r += 0.5 * rate * (y - yy) * dydr;
	delta_t += rate * (y - yy) * dydt;

	return 0;
}

 #define ALL_SEL
 #define  RSIZE 9

 #define GD_MOMENTUM

double CCurveShow::curvefit(int well, int color, int iter, int size)
{
	double delta_ko = 0;
	double delta_ro = 0;
	double delta_to = 0;

	for (int j = 0; j < iter; j++) {

		delta_k = delta_r = delta_t = 0;

#ifdef ALL_SEL

		for (int i = 3; i < size; i++) {
			jacob(x[i], y[i], k[well][color], r[well][color], t[well][color], y[size-1], fit_count[well][color]);
		}

#else	// randomly select RSIZE points for optimization

		int rsize = RSIZE;		// reduced size;
		int rindx[RSIZE];

		if (rsize > size) rsize = size;

		for (int i = 0; i < rsize; i++) {
			rindx[i] = rand() % (size - 3) + 3;
		}

		for (int i = 0; i < rsize; i++) {
			jacob(x[rindx[i]], y[rindx[i]], k[well][color], r[well][color], t[well][color], y[size - 1], fit_count[well][color]);
		}

#endif

#ifdef GD_MOMENTUM
		delta_k += 0.8 * delta_ko;
		delta_r += 0.8 * delta_ro;
		delta_t += 0.8 * delta_to;
#endif

		if ((k[well][color] > 300 && delta_k > 0) || (k[well][color] < 0 && delta_k < 0))
			delta_k = 0;

		if ((r[well][color] > 0.65 && delta_r > 0) || (r[well][color] < 0.2 && delta_r < 0))
			delta_r = 0;

		if ((t[well][color] > 50 && delta_t > 0) || (t[well][color] < 10 && delta_t < 0))
			delta_t = 0;

		k[well][color] += delta_k; 
		r[well][color] += delta_r;
		t[well][color] += delta_t;

		if (r[well][color] > 0.7)
			r[well][color] = 0.7;
		else if (r[well][color] < 0.15)
			r[well][color] = 0.15;

		fit_count[well][color] += 1;

		delta_ko = delta_k;
		delta_ro = delta_r;
		delta_to = delta_t;
	}

	double ct = t[well][color] - ct_offset[color] / r[well][color];

	if (ct > MIN_CT && ct <= size)
		m_CTValue[color][well] = ct;

	return 0;
}

void CCurveShow::OnBnClickedCheckLogscale()
{
	UpdateData(true);

/*	if (m_bLogScale) {
		(CEdit *)GetDlgItem(IDC_EDIT_LOGTH)->EnableWindow(true);
	}
	else {
		(CEdit *)GetDlgItem(IDC_EDIT_LOGTH)->EnableWindow(false);
	}
*/

	CString str;

	if (m_bLogScale) {
		m_bNormTop = true;
		m_bAlignBase = true;

		UpdateData(false);

		str.Format("%3.2f", log10(100 * log_threshold[0]));
		SetDlgItemTextA(IDC_EDIT_LOGTH, str);
		str.Format("%3.2f", log10(100 * log_threshold[1]));
		SetDlgItemTextA(IDC_EDIT_LOGTH2, str);
		str.Format("%3.2f", log10(100 * log_threshold[2]));
		SetDlgItemTextA(IDC_EDIT_LOGTH3, str);
		str.Format("%3.2f", log10(100 * log_threshold[3]));
		SetDlgItemTextA(IDC_EDIT_LOGTH4, str);
	}
	else {
		str.Format("%3.1f", (100 * log_threshold[0]));
		SetDlgItemTextA(IDC_EDIT_LOGTH, str);
		str.Format("%3.1f", (100 * log_threshold[1]));
		SetDlgItemTextA(IDC_EDIT_LOGTH2, str);
		str.Format("%3.1f", (100 * log_threshold[2]));
		SetDlgItemTextA(IDC_EDIT_LOGTH3, str);
		str.Format("%3.1f", (100 * log_threshold[3]));
		SetDlgItemTextA(IDC_EDIT_LOGTH4, str);
	}


}

void CCurveShow::OnBnClickedCheckbase()
{
	UpdateData(true);
}



void CCurveShow::OnBnClickedCheckChan1()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckChan2()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckChan3()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckChan4()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell1()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell2()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell3()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell4()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell5()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell6()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell7()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckWell8()
{
	// TODO: Add your control notification handler code here
	OnBnClickedAllcurve();
}


void CCurveShow::OnBnClickedCheckRaw()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	if (m_bRaw) {
		cheat_factor = 0;
		cheat_factor_org = 1;
		hide_org = false;

		m_bLogScale = false;
		m_bNormTop = false;
//		m_bAlignBase = false;

		UpdateData(false);
	}
	else {
		cheat_factor = 0.06;
		cheat_factor_org = 0.06;
		hide_org = true;

		m_bLogScale = false;
		m_bNormTop = true;
//		m_bAlignBase = true;

		UpdateData(false);

	}
}


void CCurveShow::OnBnClickedCheckMeltcurve()
{
	UpdateData(true);

	m_bAlignBase = false;
	m_bRaw = true;

	UpdateData(false);
}


void CCurveShow::OnEnKillfocusEditLogth()
{
	// TODO: Add your control notification handler code here
	// m_bThChange = true;

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_LOGTH, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

///	if ((fTemp > 2) | (fTemp < 0.5)) {
//		MessageBox("Please set value between 0.5 and 2");
//	}

	if ((fTemp > 50) | (fTemp < 5)) {
		MessageBox("Please set value between 5 and 50");

		SetDlgItemTextA(IDC_EDIT_LOGTH, "5");
	}

	initialize();
}


void CCurveShow::OnEnKillfocusEditLogth2()
{
	// TODO: Add your control notification handler code here
	// m_bThChange = true;

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_LOGTH2, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

//	if ((fTemp > 2) | (fTemp < 0.5)) {
//		MessageBox("Please set value between 0.5 and 2");
//	}

	if ((fTemp > 50) | (fTemp < 5)) {
		MessageBox("Please set value between 5 and 50");
	}

	initialize();
}


void CCurveShow::OnEnKillfocusEditLogth3()
{
	// TODO: Add your control notification handler code here
	// m_bThChange = true;

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_LOGTH3, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

//	if ((fTemp > 2) | (fTemp < 0.5)) {
//		MessageBox("Please set value between 0.5 and 2");
//	}

	if ((fTemp > 50) | (fTemp < 5)) {
		MessageBox("Please set value between 5 and 50");
	}

	initialize();
}


void CCurveShow::OnEnKillfocusEditLogth4()
{
	// TODO: Add your control notification handler code here
	// m_bThChange = true;

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_LOGTH4, sTemp);

	float fTemp;
	fTemp = (float)atof(sTemp);

//	if ((fTemp > 2) | (fTemp < 0.5)) {
//		MessageBox("Please set value between 0.5 and 2");
//	}

	if ((fTemp > 50) | (fTemp < 5)) {
		MessageBox("Please set value between 5 and 50");

		SetDlgItemTextA(IDC_EDIT_LOGTH4, "5");
	}

	initialize();
}


void CCurveShow::OnBnClickedCheckNormtop()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}

void  CCurveShow::ParseDPReader(CString blockData)
{
	std::regex pattern("[ \n][0-9]+");
	std::smatch result;
	std::string str(blockData.GetBuffer());
	std::string::const_iterator it = str.begin();//
	std::string::const_iterator end = str.end();//

	BYTE buff[500];

	if (std::regex_search(str, result, pattern))
	{
		int i = 0;
		while (std::regex_search(it, end, result, pattern))
		{
			CString tempNumber(result[0].str().c_str());
			if (tempNumber == "\r" || tempNumber == "\n" || tempNumber == "\r\n")continue;
			tempNumber = tempNumber.Trim();
			int value = atoi(tempNumber);

			buff[i] = (BYTE)value;
			i++;

			it = result[0].second;
		}
		
		ASSERT(g_pDPReader);
		g_pDPReader->CopyBuffAndRestore(buff, i);
	}
}
