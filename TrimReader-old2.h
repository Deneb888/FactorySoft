#define TRIM_IMAGER_SIZE 12

class CTrimNode {

public:

	double kb[TRIM_IMAGER_SIZE][4];
	double fpn[2][TRIM_IMAGER_SIZE];	// 0 - lg, 1 - hg
	unsigned int rampgen;
	unsigned int auto_v20[2];
	unsigned int auto_v15;
	double tempcal[TRIM_IMAGER_SIZE];

	CString name;

public:

	CTrimNode();
	
private:

	void Initialize();
};

#define TRIM_MAX_NODE 4
#define TRIM_MAX_WORD 640

class CTrimReader {

protected:

	CFile InFile;

	CString WordBuf[TRIM_MAX_WORD];
	int WordIndex;
	int MaxWord;
	CString CurWord;

	int calib2;

public:

	CTrimNode Node[TRIM_MAX_NODE];
	CTrimNode *curNode;
	int NumNode;

public:

	CTrimReader();
	~CTrimReader();

	int Load(TCHAR*);
	void Parse();
	void ParseNode();

	int GetNumNode() {
		return NumNode;
	}

	int ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte,  int pixelNum, int PCRNum, int gain_mode, int *flag);		// flag - overflow/underflow error code

	void SetCalib2(int c) {
		calib2 = c;
	}

protected:

	BOOL fileLoaded;

	void ParseMatrix();
	void ParseArray(int);
	void ParseValue(int);

private:

	int GetWord();
	int Match(CString);
};



