#define TRIM_IMAGER_SIZE 12
#define MAX_TRIMBUFF 256

#define EPKT_SZ  52					// Not include parity, made it 52 instead of 51 for qPCR version
#define NUM_EPKT 4

class CTrimNode {

public:

	double kb[TRIM_IMAGER_SIZE][6];		// New (auto) calib method with hump factor
	double fpn[2][TRIM_IMAGER_SIZE];	// 0 - lg, 1 - hg

	unsigned int rampgen;
	unsigned int range;
	unsigned int auto_v20[2];
	unsigned int auto_v15;
	unsigned int version;				// 0 or 1, the legacy old format with smaller matrices; 2 - new floating point format, output from autocal; 3 - integer version, ready to be written to flash

	double tempcal[TRIM_IMAGER_SIZE];

	int kbi[TRIM_IMAGER_SIZE][6];		// New (auto) calib method with hump factor, all are int
	int fpni[2][TRIM_IMAGER_SIZE];		// 0 - lg, 1 - hg

	CString name;

	BYTE	trim_buff[MAX_TRIMBUFF];
	int		tbuff_size;
	int		tbuff_rptr;

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
//	int version;

public:

	CTrimNode Node[TRIM_MAX_NODE];
	CTrimNode *curNode;
	int NumNode;
	CString TrimDotData;

public:

	CTrimReader();
	~CTrimReader();

	int Load(TCHAR*);
	void CloseFile();
	void Parse();
	void ParseNode();
	void Parse(int start_index);

	int GetNumNode() {
		return NumNode;
	}

	int ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte,  int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectionf(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectioni(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);
	int ADCCorrectionOld(int NumData, BYTE HighByte, BYTE LowByte, int pixelNum, int PCRNum, int gain_mode, int *flag);

	void SetCalib2(int c) {
		calib2 = c;
	}

	void JustParseMatrix();
	void WriteTrimDatai();
	void WriteTrimData();

	int Add2TrimBuff(int i, int);
	int Add2TrimBuff(int i, BYTE);
	int WriteTrimBuff(int i);
	int TrimBuff2Int(int i);
	BYTE TrimBuff2Byte(int i);
	void RestoreTrimBuff(int k);
	void CopyEepromBuff(int k, int index_start);

	void Convert2Int(int c);

 protected:

	BOOL fileLoaded;

	void ParseMatrix();
	void ParseArray(int);
	void ParseValue(int);
	void ParseMatrixOld();

private:

	int GetWord();
	int Match(CString);
};


#include<vector>

class DPReader {

public:

	BYTE	trim_buff[1024];
	int		tbuff_size;
	int		tbuff_rptr;

	BYTE	version;
	BYTE	id;
	BYTE	serial_number1, serial_number2;
	BYTE	num_wells, num_channels, well_format, channel_format;
	char	tc95, tc55;
//	BYTE	chan_id[MAX_CHAN];
	BYTE	num_pages;						// number of EPKT_SZ byte pages needed		
	std::string		id_str;

	std::vector <BYTE> row_index[4][16], col_index[4][16];		// Bright spots row and column, first dimenstion is channel, second dimention is well
																// The size of a given vector is determined by number of pixel for that bright spot. Row and col index same dimension
public:

	DPReader();

	void Parse();												// Read m_PositionChip 
	void ParseChannel(int);
	int WriteTrimBuff();										// 
	int Add2TrimBuff(BYTE);
	void ModTrimBuff(int pos, BYTE val);

	BYTE TrimBuff2Byte();
	void CopyEepromBuffAndRestore();

	CString WriteDPIniFile();

	void regDispatch(CString);

	void CopyBuffAndRestore(BYTE buff[], int size);
	void RestoreFromTrimBuff1();
	void RestoreFromTrimBuff2();
};



