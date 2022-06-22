#pragma once

#include <vector>
#include "chart\ChartCtrl\ChartLineSerie.h"
#include "chart\chartctrl\chartctrl.h"
#include "GraDlg.h"

// CCurveShow
/*
class temp_model {

public:

	float x, x0, y, dt, drive, xd, yd;
	float init_temp;
	float c, c2, kp, ki, l, sige;
	float xdl1, xdl2, xdl3, xdl4;

	float h_bd, l_bd;

	float dn_temp, dn_time, an_temp, an_time, ex_temp, ex_time;

	float t;

	std::vector <double> v_x, v_t, v_s;

	temp_model();
	void simdt();
	void sim_step(float temp, float time);
};
*/

extern 	std::vector<double> m_yData[MAX_CHAN][MAX_WELL];	//handled data，4 channels，4 frames

extern	int frame_data[24][24];		// A whole frame of data
extern std::vector <int> v_frame_data[4][12];

extern	std::vector<std::string> m_PositionChip[4];
extern int numWells;


#define MAX_CYCL 400

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
	afx_msg void OnBnClickedFrame5();
	afx_msg void OnBnClickedFrame6();
	afx_msg void OnBnClickedFrame7();
	afx_msg void OnBnClickedFrame8();

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	bool isInitialized;
	CChartAxis *pAxisBottom;
	CChartAxis *pAxisLeft; 

	void CalculateCT(void);
	double m_CTValue[4][MAX_WELL];
	double IsCT(void);
	void DrawCTLine(int frameindex);

	int datasize;
	double biggestvalue;

	CString printVector(std::vector<double> list);
	afx_msg void OnBnClickedOpencurvefile();
	int ReadDataFromFile(CString filePath);
private:
	void splitStringByBlank(CStdioFile * file, int index);
public:
//	void regDispatch(void);
//	std::vector<std::string> m_PositionChip[4];
	void UpdatePCRCurveData(int PCRNum, int pixelNum);
	void initialize(void);
	afx_msg void OnBnClickedAllcurve();
	void PostNcDestroy();
	virtual void OnCancel();

	void DrawSigCurve(int frameindex);
	CChartLineSerie *m_pSigCurveSerie[4];		//sigmoid curve

	void DrawMeltCurve(int frameindex);

	double sigmoid(double x, double k, double r, double t);
	double jacob(double x, double y, double k, double r, double t, double endy, int fit_count);
	double curvefit(int well, int color, int iter, int size);

	double	x[MAX_CYCL], y[MAX_CYCL];
	double	k[MAX_WELL][4], r[MAX_WELL][4], t[MAX_WELL][4];
	double	delta_k, delta_r, delta_t;
	long	fit_count[MAX_WELL][4];

//	float ct_offset; //  , log_threshold;  // log threshold is now global

	void UpdateThreshold();

	// Log scale display
	BOOL m_bLogScale;
	afx_msg void OnBnClickedCheckLogscale();
	// Align the bases of all curves
	BOOL m_bAlignBase;
	afx_msg void OnBnClickedCheckbase();

	CStdioFile	*m_pSFile;
	BOOL		m_bSFileOpen;

	CStdioFile file;

	CGraDlg *m_pGra;

	afx_msg void OnBnClickedButtonTcurve();
	afx_msg void OnBnClickedCheckChan1();
	afx_msg void OnBnClickedCheckChan2();
	afx_msg void OnBnClickedCheckChan3();
	afx_msg void OnBnClickedCheckChan4();
	afx_msg void OnBnClickedCheckWell1();
	afx_msg void OnBnClickedCheckWell2();
	afx_msg void OnBnClickedCheckWell3();
	afx_msg void OnBnClickedCheckWell4();
	afx_msg void OnBnClickedCheckWell5();
	afx_msg void OnBnClickedCheckWell6();
	afx_msg void OnBnClickedCheckWell7();
	afx_msg void OnBnClickedCheckWell8();
	BOOL m_bRaw;
	afx_msg void OnBnClickedCheckRaw();
	BOOL m_MeltCurve;
	afx_msg void OnBnClickedCheckMeltcurve();

	void SetGraDlg(CGraDlg *g) {
		m_pGra = g;
	}
};


