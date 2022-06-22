#define TRIM_IMAGER_SIZE 12

class CTrimNode {

public:

	double kb[TRIM_IMAGER_SIZE][2];
	double fpn[2][TRIM_IMAGER_SIZE];	// 0 - lg, 1 - hg
	unsigned int rampgen;
	unsigned int auto_v20[2];
	double tempcal[TRIM_IMAGER_SIZE];

	CString name;

public:

	CTrimNode();
	
private:

//	void Initialize();
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

protected:

	BOOL fileLoaded;

	void ParseMatrix();
	void ParseArray(int);
	void ParseValue(int);

private:

	int GetWord();
	int Match(CString);
//	int IsSpace(char);
};



