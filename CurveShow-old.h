#pragma once

#include <vector>
#include "chart\ChartCtrl\ChartLineSerie.h"
#include "chart\chartctrl\chartctrl.h"
// CCurveShow 对话框

extern 	std::vector<double> m_yData[4][4];	//handled data，4 channels，4 frames
extern	int frame_data[24][24];		// A whole frame of data

class CCurveShow : public CDialogEx
{
	DECLARE_DYNAMIC(CCurveShow)

public:
	CCurveShow(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCurveShow();

// 对话框数据
	enum { IDD = IDD_CURVESHOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	std::vector<double> m_x;				//x axis
	void initChart(void);
	void ResetChart(int chipindex);
	CChartLineSerie *m_pLineSerie[4];		//chart line
	CChartLineSerie *m_pCTLineSerie[4];		//chart line
	CChartCtrl m_chart;
	afx_msg void OnBnClickedFrame1();
	afx_msg void OnBnClickedFrame2();
	afx_msg void OnBnClickedFrame3();
	afx_msg void OnBnClickedFrame4();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	bool isInitialized;
	CChartAxis *pAxisBottom;
	CChartAxis *pAxisLeft; 

	void CalculateCT(void);
	double m_CTValue[4][4];
	int IsCT(void);
	void DrawCTLine(int frameindex);
	int datasize;
	double biggestvalue;
	CString printVector(std::vector<double> list);
	afx_msg void OnBnClickedOpencurvefile();
	int ReadDataFromFile(CString filePath);
private:
	void splitStringByBlank(CStdioFile * file, int index);
public:
	void regDispatch(void);
	std::vector<std::string> m_PositionChip[4];
	void UpdatePCRCurveData(int PCRNum, int pixelNum);
	void initialize(void);
	afx_msg void OnBnClickedAllcurve();
	void PostNcDestroy();
	virtual void OnCancel();
};
