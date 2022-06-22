
#include "stdafx.h"
#include "TrimReader.h"

// Node

CTrimNode::CTrimNode()
{
//	Initialize();
}

/*
void 
CTrimNode::Initialize()
{
}


void
CTrimNode::SetName(const char *n) 
{ 
	strcpy_s(name, n);
}
*/

// Reader

CTrimReader::CTrimReader() 
{ 
//	InFile = 0; 
	curNode = NULL;
	NumNode = 0;

	WordIndex = 0;
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

	DWORD fl = InFile.GetLength();

	char *buf = new char[fl];

	InFile.Read(buf, fl);

	FileBuf = buf;

	delete buf;

	int ep;
//	CString wbuf[1000];
	CString delimit = CString(", \t\r\n");

	int i = 0;

//	wbuf = FileBuf.SpanExcluding(delimit);

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

/*
int CTrimReader::IsSpace(char byte)
{
	if( byte == '\n' 
	 || byte == '\0' 
	 || byte == ',' 
	 || byte == ' ' 
	 || byte == '\t'
	 || byte == 13
	 ) return 1;

	return 0;
}
*/

int CTrimReader::Match(CString s)
{
//	int m;
//	m =  strcmp(s1, s2);
//	return (m == 0);

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
//				GetWord();
//				if(!Match(CString("}"))) 
//					return;
			}
			else break;
		}
		else break;
	}

	NumNode = i;
//	ParseNode();
}


void CTrimReader::ParseNode()
{
//	char string[128];
//	char name[64];

//	name[0] = '\0';

//	CString Name;

	if(!InFile) 
		return;

//	GetWord();

	for(;;) {		
		if(GetWord() == MaxWord) 
			break;		
		
//		if(Match(CString("DEF"))) {
//			GetWord();
//			Name = CurWord;
//		}
//		else 
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
/*		else if(Match(string, "Separator")) {
			GetWord(string);
			if(Match(string, "{")) {
				indx++;
				curNode->AddChild(Node + indx);
				curNode = Node + indx;
				strcpy_s(Name[indx], name);
				NumNode++;
				ParseNode();	
			}
			else return;
		}*/
		else if(Match(CString("}"))) {
			return;
		}
		else 
			return;
	}
}



void CTrimReader::ParseMatrix()
{
//	char string[128];

//	if(!InFile) 
//		return;

//	if(GetWord(string) == EOF) 
//			return;

//	if(!Match(string, "kbmatrix")) 
//		return;

	for(int i=0; i<TRIM_IMAGER_SIZE; i++) {
		for(int j=0; j<2; j++) {
			if(GetWord() == MaxWord) 
				break;
			curNode->kb[i][j] = _tstof((LPCTSTR)CurWord); // atof(CurWord);
		}
	}
}


void CTrimReader::ParseArray(int gain) 
{
//	char string[128];

//	if(!InFile) 
//		return;

//	if(GetWord(string) == EOF) 
//			return;

//	if(!Match(string, "array")) 
//		return;

//	if(GetWord(string) == EOF) 
//			return;

//	if(!Match(string, "[")) 
//		return;

//	int index = 0;
//	double x;

	for(int i=0; i<12; i++) {
		GetWord();
//		if(Match(string, "]")) {
//			curNode->nv = index;
//			break;
//		}

//		sscanf_s(string, "%lf", &x);		
		if(gain == 2) 
			curNode->tempcal[i] = _tstof((LPCTSTR)CurWord);
		else 
			curNode->fpn[gain][i] = _tstof((LPCTSTR)CurWord);
	}
}

 void CTrimReader::ParseValue(int gain) 
{
//	char string[128];

//	if(!InFile) 
//		return;

//	if(GetWord(string) == EOF) 
//			return;

//	unsigned int x;

//	sscanf_s(string, "%x", &x);	'
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
	else 
		curNode->auto_v20[gain] = val;

}


