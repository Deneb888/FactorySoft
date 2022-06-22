
#include "stdafx.h"
#include "TrimReader.h"

#define SAW_TOOTH2			// Newer Sawtooth algorithm. USe 2 pass low byte correction
#define NON_CONTIGUOUS

// Node

CTrimNode::CTrimNode()
{
	Initialize();
}


void 
CTrimNode::Initialize()
{
	int i;

	for(i=0; i<TRIM_IMAGER_SIZE; i++) {
		kb[i][0] = 1;
		kb[i][1] = 0;
		kb[i][2] = 0;
		kb[i][3] = 0;

		fpn[0][i] = 0;
		fpn[1][i] = 0;

		if(!i) tempcal[i] = 1;
		else tempcal[i] = 0;
	}

	rampgen = 0x88;
	
	auto_v20[0] = 0x8;
	auto_v20[1] = 0x8;

	auto_v15 = 0x8;
}


// Reader

CTrimReader::CTrimReader() 
{ 
//	InFile = 0; 
	curNode = NULL;
	NumNode = 0;

	WordIndex = 0;

	calib2 = 0;
}

CTrimReader::~CTrimReader() 
{
	if(fileLoaded) InFile.Close();
}

int CTrimReader::Load(TCHAR* fn) 
{
	int e;
	CString FileBuf;

	e = InFile.Open(fn, CFile::modeRead);

	fileLoaded = e;

	if(!e) return e;

	DWORD fl = (DWORD)InFile.GetLength();

	char *buf = new char[fl];

	InFile.Read(buf, fl);

	FileBuf = buf;

	delete buf;

	int ep;

	CString delimit = CString(", \t\r\n");

	int i = 0;

	FileBuf.TrimLeft(delimit);

	while (((ep = FileBuf.FindOneOf(delimit)) != -1) && i < TRIM_MAX_WORD) 
	{
		WordBuf[i] = FileBuf.Mid(0, ep);
		int l = FileBuf.GetLength();
		FileBuf = FileBuf.Mid(ep, (l - ep));
		FileBuf.TrimLeft(delimit);
		i++;
	}
	
	MaxWord = i;
	FileBuf.Empty();

	return e;
}

int CTrimReader::GetWord()
{
	CurWord = WordBuf[WordIndex];
	WordIndex++;

	return WordIndex;
}

int CTrimReader::Match(CString s)
{
	return (int)(CurWord.Compare(s) == 0);
}

void CTrimReader::Parse()
{
	CString Name;
	int i = 0;

	if(!InFile) 
		return;

	for(;;) {		
		if(GetWord() == MaxWord) 
			break;		
		
		if(Match(CString("DEF"))) {
			GetWord();
			Name = CurWord;
			
			GetWord();
			if(Match(CString("{"))) {
				curNode = Node + i;
				i++;
				curNode->name = Name;
				ParseNode();
			}
			else break;
		}
		else break;
	}

	NumNode = i;
}


void CTrimReader::ParseNode()
{
	if(!InFile) 
		return;

	for(;;) {		
		if(GetWord() == MaxWord) 
			break;		
		
		if(Match(CString("Kb"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseMatrix();

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("Fpn_lg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(0);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}		
		else if(Match(CString("Fpn_hg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(1);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("Temp_calib"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseArray(2);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}		
		else if(Match(CString("Rampgen"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(2);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV20_lg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(0);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV20_hg"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(1);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("AutoV15"))) {
			GetWord();
			if(Match(CString("{"))) {
				ParseValue(3);

				GetWord();
				if(!Match(CString("}"))) 
					return;
			}
			else return;
		}
		else if(Match(CString("}"))) {
			return;
		}
		else 
			return;
	}
}

void CTrimReader::ParseMatrix()
{
	for(int i=0; i<TRIM_IMAGER_SIZE; i++) {
		for(int j=0; j<4; j++) {
			if(GetWord() == MaxWord) 
				break;
			curNode->kb[i][j] = _tstof((LPCTSTR)CurWord); // atof(CurWord);
		}
	}
}


void CTrimReader::ParseArray(int gain) 
{
	for(int i=0; i<12; i++) {
		GetWord();	
		if(gain == 2) 
			curNode->tempcal[i] = _tstof((LPCTSTR)CurWord);
		else 
			curNode->fpn[gain][i] = _tstof((LPCTSTR)CurWord);
	}
}

// gain: 0, 1 - auto_v20[0, 1]; 2: rampgen; 3: auto_v15

 void CTrimReader::ParseValue(int gain) 
{
	GetWord();

	CString word = CurWord;
	word.MakeLower();
	int p = word.Find(CString("0x"));
	int l = word.GetLength();
	word = word.Mid(p+2, l-p-2);
	unsigned int val = _tstoi((LPCTSTR)word);

	val =  _tcstoul((LPCTSTR)word, 0, 16);

	if(gain == 2)
		curNode->rampgen = val;
	else if(gain == 3) 
		curNode->auto_v15 = val;
	else 
		curNode->auto_v20[gain] = val;
}

#define DARK_LEVEL 100
#define DARK_MANAGE
 
 // NumData =  "Column Number"
 // pixekNum = "Frame Size"

 // With the Sawtooth method, we need to gather denser data and perform a better data fitting.

 int CTrimReader::ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int* flag)
{
	int hb,lb, lbc;
	int hbln,lbp,hbhn;
	bool oflow = false, uflow = false; //  qerr_big=false;

//	CString strbuf;
	double ioffset = 0; 
	int result;

	hb = (int)HighByte;

	int nd = 0;
	if(pixelNum == 12) nd = NumData;
	else nd = NumData >> 1;

	ioffset = Node[PCRNum-1].kb[nd][0] * (double)hb + Node[PCRNum-1].kb[nd][1];

#ifdef NON_CONTIGUOUS

	if(hb >= 128) {
		ioffset += Node[PCRNum-1].kb[nd][3];
	}

#endif

	hbln = hb % 16;		//

	hbhn = hb / 16;		//
		
#ifdef SAW_TOOTH		

		ioffset += Node[PCRNum-1].kb[nd][2] * (hbln - 7);

#endif

//		ioffset = Node[PCRNum-1].kb[nd][0]*hb + Node[PCRNum-1].kb[nd][1] + Node[PCRNum-1].kb[nd][2] *(hbln - 7);

	lb = (int)LowByte;

	lbc = lb + (int)ioffset;

#ifdef SAW_TOOTH2							// Use lbc, not hbln to calculate sawtooth correction, as hbln tends to be a little unstable	
		ioffset += Node[PCRNum-1].kb[nd][2] * ((double)lbc - 127) * (1 - (double)hb / 400) / 16;		// 12/19/2016 modification, shrinking sawtooth.
		lbc = lb + (int)ioffset;					// re-calc lbc, 2 pass algorithm
#endif
		
	lbp = hbln * 16 + 7;
		
	if(lbc > 255) lbc = 255;
	else if(lbc < 0) lbc = 0;
	
	int lbpc = lbp - (int)ioffset;				// lpb - ioffset: low byte predicted from the high byte low nibble BEFORE correction
	int qerr = lbp - lbc;					// if the lbc is correct, this would be the quantization error. If it is too large, maybe lb was the saturated "stuck" version
		
	if (lbpc > 255 + 20) {					// We allow some correction error, because hbln may have randomly flipped.
		oflow = true; *flag = 1;
	}
	else if (lbpc > 255 && qerr > 28) {		// Again we allow some tolerance because hbln may have drifted, leading to fake error
		oflow = true; *flag = 2;
	}
	else if(lbpc > 191 && qerr > 52) {
		oflow = true; *flag = 3;
	}
	else if(qerr > 96) {
		oflow = true; *flag = 4;
	}
	else if(lbpc < -20) {
		uflow = true; *flag = 5;
	}
	else if(lbpc < 0 && qerr < -28) {
		uflow = true; *flag = 6;
	}
	else if(lbpc < 64 && qerr < -52) {
		uflow = true; *flag = 7;
	}
	else if(qerr < -96) {
		uflow = true; *flag = 8;
	}
	else {
		*flag = 0;
	}
		
//	if(abs(qerr) > 84) qerr_big = true;
		
	if (oflow || uflow) {
		result = hb * 16 + 7;
	}
	else {
		result = hbhn * 256 + lbc;
	}		

	if(calib2) return result;
		
#ifdef DARK_MANAGE

	if(!gain_mode)
		result += -(int) (Node[PCRNum-1].fpn[1][nd]) + DARK_LEVEL;		// high gain
	else 
		result += -(int) (Node[PCRNum-1].fpn[0][nd]) + DARK_LEVEL;		// low gain

	if(result < 0) result = 0;

#endif

	return result;
}
 
