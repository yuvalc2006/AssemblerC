#ifndef FIRSTSWEEP_H
#define FIRSTSWEEP_H


typedef struct Label Label;

typedef struct dataTable dataTable;


struct Label {
    char *name;
    /* line should represent the line that the label holds */
    int line;
    /* 'd' represents a data line, 'e' represents an external label and 'c' represents a command line */
    char type;
    int isEntry;
    Label *next;
};

struct dataTable {
    int data;
    dataTable *next;
};

int firstSweep(char fileName[]);

int isLineLabelDec(char currLine[]);

int isLineDataDec(char currLine[]);

int addFuncLabel(char *name);

int addDataLabel(char *name);

Label *isLabel(char currWord[]);

void removeColon(char currWord[]);

void addData(char currLine[]);

int isValidLabelName (char labelName[]);

void strcpyPart(char *destination, const char *origin, int start, int end);

void addDataToList(int data);

int entryOrExtern(char currLine[], char currWord[]);

int isExtern (char currLine[]);

void getFuncAndOps (char currLine[], char currWord[], char func[], char op1[], char op2[]);

int isNumber(char str[]);

int isDigit (char c);

const char *skipWhitespace(const char *str);

void addExternals(char currLine[], int index);

int isRealFunc(char currWord[]);

int computeL(char func[], const char op1[], const char op2[]);

void goNextWord(const char *currLine, char *currWord);

void addDataAndString(char currLine[], char currWord[]);

char* binaryOfFunc(char func[]);

char *opCode(char op[]);

void addICToData();

#endif
