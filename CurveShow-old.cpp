// CurveShow.cpp : 实现文件
//

#include "stdafx.h"
#include "PCRProject.h"
#include "CurveShow.h"
#include "afxdialogex.h"
#include "chart\fit.h"
#include "chart\ChartCtrl\ChartCtrl.h"
#include<numeric>
#include<regex>
#include "GraDlg.h"

extern CCurveShow *pCVS; // pointer to self

// CCurveShow

IMPLEMENT_DYNAMIC(CCurveShow, CDialogEx)

CCurveShow::CCurveShow(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCurveShow::IDD, pParent)
{
	initialize();
}

CCurveShow::~CCurveShow()
{
}

void CCurveShow::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHART, m_chart);
}


BEGIN_MESSAGE_MAP(CCurveShow, CDialogEx)
	ON_BN_CLICKED(IDC_FRAME1, &CCurveShow::OnBnClickedFrame1)
	ON_BN_CLICKED(IDC_FRAME2, &CCurveShow::OnBnClickedFrame2)
	ON_BN_CLICKED(IDC_FRAME3, &CCurveShow::OnBnClickedFrame3)
	ON_BN_CLICKED(IDC_FRAME4, &CCurveShow::OnBnClickedFrame4)
	ON_BN_CLICKED(IDOK, &CCurveShow::OnBnClickedOk)
	ON_BN_CLICKED(IDC_OPENCURVEFILE, &CCurveShow::OnBnClickedOpencurvefile)
	ON_BN_CLICKED(IDC_ALLCURVE, &CCurveShow::OnBnClickedAllcurve)
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
	isInitialized=true;
}
void CCurveShow::ResetChart(int frameindex)
{
	m_chart.RemoveAllSeries();	//
	biggestvalue=0;
	datasize=m_yData[0][0].size()>0?m_yData[0][0].size():
		(m_yData[1][0].size()>0?m_yData[1][0].size():
		(m_yData[2][0].size()>0?m_yData[2][0].size():
		(m_yData[3][0].size()>0?m_yData[3][0].size():0)));
	if(datasize<5)
	{
		AfxMessageBox("The data isn't ready. Please check it");
		return ;
	}
	m_x.resize(datasize);

	for(size_t i =0;i<datasize;++i)
	{
		m_x[i] = i;
	}
	
	for(int iy=0;iy<4;iy++)
	{
		std::vector<double> yData=m_yData[iy][frameindex];	
		if(yData.size()<=5)continue;
#ifdef _DEBUG
		printVector(yData);					//print yData
#endif

		czy::Fit fit;
		//fit.polyfit(m_x,yData,30,true);
		//std::vector<double> yploy;
		//fit.getFitedYs(yploy);
		//yploy.assign(yData.begin(),yData.end());
		std::vector<double>::iterator biggest=std::max_element(std::begin(yData),std::end(yData));
		biggestvalue=biggestvalue>=(*biggest)?biggestvalue:(*biggest);
		m_pLineSerie[iy] = m_chart.CreateLineSerie();
		m_pLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
		m_pLineSerie[iy]->AddPoints(&m_x[0], &yData[0], yData.size());
		switch(iy)
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
			m_pLineSerie[iy]->SetName(_T("UNKNOWN"));
			break;
		}

		m_pLineSerie[iy]->SetWidth(2);
		m_pLineSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
		m_pLineSerie[iy]->SetVisible(true);
	}
	pAxisLeft->SetMinMax(0,biggestvalue+200);
	m_chart.RefreshCtrl();
}
// CCurveShow 
void CCurveShow::OnBnClickedFrame1()
{
	ResetChart(0);
	CalculateCT();
	DrawCTLine(0);
}


void CCurveShow::OnBnClickedFrame2()
{
	ResetChart(1);
	CalculateCT();
	DrawCTLine(1);
}


void CCurveShow::OnBnClickedFrame3()
{
	ResetChart(2);
	CalculateCT();
	DrawCTLine(2);
}


void CCurveShow::OnBnClickedFrame4()
{
	ResetChart(3);
	CalculateCT();
	DrawCTLine(3);
}


BOOL CCurveShow::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	if(!isInitialized)
		initChart();
	OnBnClickedAllcurve();
	return true;
}


void CCurveShow::OnBnClickedOk()
{
	m_chart.RemoveAllSeries();//先清空
	UpdateData(true);			// Zhimin added
	CDialogEx::OnOK();
	DestroyWindow();			// Zhimin added
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



void CCurveShow::CalculateCT(void)
{
	if(IsCT()>0)return;//CT valus have been calculated,need not been calculated again.

	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			std::vector<double> tempData;
			if(m_yData[i][j].size()<15)//data has not enough
				continue;
			tempData.assign(m_yData[i][j].begin()+3,m_yData[i][j].begin()+15);
			//calculate CT value
			double sum = std::accumulate(std::begin(tempData), std::end(tempData), 0.0);
		
			double mean =  sum / tempData.size(); //mean 
     
			double accum  = 0.0;  
			std::for_each (std::begin(tempData), std::end(tempData), [&](const double d) {  
				accum  += (d-mean)*(d-mean);  
			});  

			double stdev = sqrt(accum/tempData.size()); //方差
			double yvalue=stdev*10+mean;
			
			double first=0;
			int index=0;
			std::vector<double>::iterator it=std::find_if(m_yData[i][j].begin()+3,m_yData[i][j].end(),[&](const double d) {  
				if(yvalue>first && yvalue<=d)return true;
				else{first=d;index++;return false;}
			});

			if(index==0||it==m_yData[i][j].end())
			{
				m_CTValue[i][j]=0;
#ifdef _DEBUG
				CString debuginfo;
				debuginfo.Format("yvalue=%f, found the %d'th data",yvalue,index+3);
				AfxMessageBox(debuginfo);
#endif
			}
			else{
				index=index+3;
				while((*it)-(*(it-1))==0)it++;
				if((*it)-(*(it-1))!=0){
					m_CTValue[i][j]=index-((*it)-yvalue)/((*it)-(*(it-1)));
					m_CTValue[i][j]=m_CTValue[i][j]>0?m_CTValue[i][j]:0;
				}else
					m_CTValue[i][j]=0;
			}
		}
	}
}


int CCurveShow::IsCT(void)
{
	double sum=0;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			sum+=m_CTValue[i][j];
		}
	}
	return sum;
}


void CCurveShow::DrawCTLine(int frameindex)
{
	for(int iy=0;iy<4;iy++)
	{
		double x[2],y[2];
		x[0]=m_CTValue[iy][frameindex];
		x[1]=m_CTValue[iy][frameindex];
		y[0]=0;
		y[1]=biggestvalue;

		CString CTValue("Frame");
		CTValue.AppendFormat(" %dCT:%.5f",frameindex+1,m_CTValue[iy][frameindex]);
		m_pCTLineSerie[iy] = m_chart.CreateLineSerie();
		m_pCTLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
		m_pCTLineSerie[iy]->AddPoints(x, y, 2);
		m_pCTLineSerie[iy]->SetName(CTValue.GetBuffer());
		m_pCTLineSerie[iy]->SetWidth(1);
		m_pCTLineSerie[iy]->SetPenStyle(2);
		m_pCTLineSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
		m_pCTLineSerie[iy]->SetVisible(true);
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


void CCurveShow::OnBnClickedOpencurvefile()
{
	CString filter;  
	CString strFilePath;
	filter="Text File(*.txt)|*.txt|Data File(*.dat)|*.dat||";  
	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY,filter);  
	if(fileDlg.DoModal()==IDOK)  
	{   
		strFilePath=fileDlg.GetPathName();  
	}else{
		//MessageBox(strFilePath);  
		return;
	}
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++){
			m_yData[i][j].clear();
		}
	}
	for(int i=0;i<4;i++)
		m_PositionChip[i].clear();
	regDispatch();
	ReadDataFromFile(strFilePath);

	initialize();

	OnBnClickedAllcurve();
}


int CCurveShow::ReadDataFromFile(CString filePath)
{
	CStdioFile file;
	if(!file.Open(filePath,CFile::modeRead))
	{
		exit(0);
	}
	CString tempdata;
	while(file.ReadString(tempdata))
	{
		if(tempdata.Find("Chip")==-1)continue;
		if(tempdata.Find("#1")>=0)
		{
			splitStringByBlank(&file,1);
		}else if(tempdata.Find("#2")>=0)
		{
			splitStringByBlank(&file,2);
		}else if(tempdata.Find("#3")>=0)
		{
			splitStringByBlank(&file,3);
		}else{
			continue;
		}
	}
	file.Close();

	return 0;
}


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
	std::regex pattern("[ \n][0-9]+");
	std::smatch result;
	std::string str(blockData.GetBuffer());
	std::string::const_iterator it=str.begin();//取头 
	std::string::const_iterator end=str.end();//取尾
	if(std::regex_search(str,result,pattern)) 
	{
		int i=0,j=0;
		while (std::regex_search(it,end,result,pattern))
		{
			CString tempNumber(result[0].str().c_str());
			if(tempNumber=="\r"||tempNumber=="\n"||tempNumber=="\r\n")continue;
			tempNumber=tempNumber.Trim();
			int value=atoi(tempNumber);
			if(value>=0&&value<=23){
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
	UpdatePCRCurveData(index,line);
}


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


void CCurveShow::UpdatePCRCurveData(int PCRNum, int pixelNum)
{
	for(int frame=0;frame<4;frame++)
	{
		int j=0;
		int column[10],row[10];
		memset(column,0,10*sizeof(int));
		memset(row,0,10*sizeof(int));
		CString temp(m_PositionChip[PCRNum-1].at(frame).c_str());
		CString number;
		for(int i=0;i<m_PositionChip[PCRNum-1].at(frame).size();i++)
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
		double sum=0;
		for(int i=0;i<=j;i++)
		{
			sum+=frame_data[row[i]][column[i]];
		}
		m_yData[PCRNum-1][frame].push_back(sum);
	}
}


void CCurveShow::initialize(void)
{
	for(int i=0;i<4;i++)
	{
		m_pLineSerie[i]=NULL;
		for(int j=0;j<4;j++){
			m_CTValue[i][j]=0;
			if(m_yData[i][j].size()>0)
				m_yData[i][j].insert(m_yData[i][j].begin(),m_yData[i][j].at(0));//add the first data
		}
	}

	isInitialized=false;
}


void CCurveShow::OnBnClickedAllcurve()
{
	m_chart.RemoveAllSeries();//先清空
	biggestvalue=0;
	datasize=m_yData[0][0].size()>0?m_yData[0][0].size():
		(m_yData[1][0].size()>0?m_yData[1][0].size():
		(m_yData[2][0].size()>0?m_yData[2][0].size():
		(m_yData[3][0].size()>0?m_yData[3][0].size():0)));
	if(datasize<5)
	{
		return ;
	}
	m_x.resize(datasize);

	for(size_t i =0;i<datasize;++i)
	{
		m_x[i] = i;
	}
	
	for(int iy=0;iy<4;iy++)
	{
		for(int frameindex=0;frameindex<4;frameindex++){
			std::vector<double> yData=m_yData[iy][frameindex];	
			if(yData.size()<=5)continue;
			czy::Fit fit;
			std::vector<double>::iterator biggest=std::max_element(std::begin(yData),std::end(yData));
			biggestvalue=biggestvalue>=(*biggest)?biggestvalue:(*biggest);
			m_pLineSerie[iy] = m_chart.CreateLineSerie();
			m_pLineSerie[iy]->SetSeriesOrdering(poNoOrdering);//设置为无序
			m_pLineSerie[iy]->AddPoints(&m_x[0], &yData[0], yData.size());
			CString lineName("Frame");
			lineName.AppendFormat("%d-",frameindex+1);
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
				lineName.Append(_T("UNKNOWN"));
				break;
			}
			m_pLineSerie[iy]->SetName(lineName.GetBuffer());
			m_pLineSerie[iy]->SetWidth(2);
			m_pLineSerie[iy]->SetColor(RGB(iy*255/4+frameindex*255/4,
				255-iy*255/4-frameindex*255/4,(iy*255/4+128)%255));
			m_pLineSerie[iy]->SetVisible(true);
		}
	}

	CalculateCT();
	for(int frameindex=0;frameindex<4;frameindex++)
		DrawCTLine(frameindex);
	m_chart.RefreshCtrl();
	pAxisLeft->SetMinMax(0,biggestvalue+200);

}


