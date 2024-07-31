#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include "firstsweep.h"
#include "main.h"
#include "constants.h"
#include "globals.h"

/*
 * this file is responsible for the "first pass" process of storing the labels and the data
 */

int firstSweep(char fileName[]) {

    if (isErrorHappened)
        return 0;
    /* the steps I'll be referring to in this file's comments are the steps provided by the algorithm in page 36 of the course textbook */
    lineNum = 0;
    /* step 1 (we're supposed to initialize IC to 0 but we treat the memory like it starts from 100, so it makes more sense to initialize IC to 100) */
    IC = 100;
    DC = 0;

    char currLine[MAXLENGTHLINE], currWord[MAXLENGTHLINE], func[5], op1[MAXLENGTHLINE], op2[MAXLENGTHLINE];

    replaceExtension(fileName, "txt");
    FILE *assFile = fopen(fileName, "r");
    if (!assFile) {
        raiseError("Could not open input file.", 0);
        return 0;
    }

    int isLabelDeclaration, index, externOrEntryFlag;
    /* Process each line in the input assembly file (step 2) */
    while (nextLine(currLine, assFile)) {
	index = nextArg(currLine, 0);

        if (currLine[index] == ';' || currLine[index] == '\n' || currLine[index] == '\0'){
            continue;
	}


        op1[0] = '\0';
        op2[0] = '\0';
        
	/* isLabelDeclaration is the flag in step 4 and the condition in step 3 */
        isLabelDeclaration = isLineLabelDec(currLine);
	externOrEntryFlag = entryOrExtern(currLine, currWord);

	goNextWord (currLine, currWord);

        /* step 5 */
        if (isLineDataDec(currLine)) {
            /* step 6 */
            if (isLabelDeclaration) {
                addDataLabel(currWord);
                goNextWord(NULL, currWord);
            }
            /* step 7 */
            addDataAndString(currLine, currWord);
            continue;
        }

        /* step 8 */
        if (externOrEntryFlag) {

	    if (isLabelDeclaration){
		raiseWarning("a label shouldn't be defined in a \"extern\" or \"entry\" line", lineNum);
		goNextWord(NULL, currWord);
	    }
		
            /* step 9 */
            if (externOrEntryFlag == 1) {
                /* because the line is of the form ".extern label1 , label2, ..." we know label1 starts at index 8 */
                addExternals(currLine, 8);
            }
            /* step 10 */
            continue;
        }

        /* step 11 */
        if (isLabelDeclaration) {
            addFuncLabel(currWord);
            goNextWord(NULL, currWord);
        }
        /* step 12 */
        if (!isRealFunc(currWord)){
	    raiseError("unknown keyword(s)", lineNum);
	    continue;
	}
            

        /* we'll store the function's and the operands' names in strings */
        getFuncAndOps(currLine, currWord, func, op1, op2);

        /* step 13 and 14, the binary code will be created in the second pass */
        IC += computeL(func, op1, op2);

        /* because we're in a loop step 15 happens automatically */
    }

    /* step 16 */
    if (isErrorHappened)
        return 1;

    /* step 17 */
    addICToData();

    fclose(assFile);

    /* step 18 happens once we return to main */

    return 0;
}

/* get the function and it's operands */
void getFuncAndOps(char currLine[], char currWord[], char func[], char op1[], char op2[]) {
    safeStrCpy(func, currWord, MAXLENGTHLINE);

    if (strncmp(func, "stop", 4) == 0)
        func[4] = '\0';
    else
        func[3] = '\0';

    goNextWord(NULL, currWord);

    /* if "currWord" is empty then there aren't any operands in this line */
    if (currWord[0] == '\0'){
	op1[0] = '\0';
	op2[0] = '\0';
	return;
    }

    /* if "currWord" is a comma then this line doesn't follow the valid format */
    if (currWord[0] == ','){
	raiseError("The command line should follow the format \"func op1, op2\" or \"func op1\" or \"func\", but a comma was found between the function and the first operand.", lineNum);
	op2[0] = '\0';
	return;
    }

    /* we've reached an operand, we'll copy it into op1 */
    safeStrCpy(op1, currWord, MAXLENGTHLINE);


    /* now look for a comma between both operands or the end of the line (in that case there's only one operand */
    goNextWord(NULL, currWord);

    /* if "currWord" is empty then there's one operand in this line */
    if (currWord[0] == '\0'){
	op2[0] = '\0';
	return;
    }

    
    if (currWord[0] != ','){
	raiseError("The command line should follow the format \"func op1, op2\" or \"func op1\" or \"func\", but no comma was found between the operands.", lineNum);
    	return;
    }


    /* we've reached a comma, valid format so far, go to the next word */
    goNextWord(NULL, currWord);

    /* if "currWord" is empty then there's one operand in this line, however we found a comma so there should be 2 operands and so we've reached an error */
    if (currWord[0] == '\0'){
	raiseError("Found a comma after first operand so expected 2 operands but only 1 was given", lineNum);
	return;
    }

    /* if "currWord" is a comma then this line doesn't follow the valid format */
    if (currWord[0] == ','){
	raiseError("The command line should follow the format \"func op1, op2\" or \"func op1\" or \"func\", but 2 commas was found between the operands.", lineNum);
	op2[0] = '\0';
	return;
    }


    /* we've reached an operand, we'll copy it into op2 */
    safeStrCpy(op2, currWord, MAXLENGTHLINE);

    /* we've finished, however we'll check if there's still more words in this line */
    goNextWord(NULL, currWord);

    if (currWord[0] != '\0')
	raiseError("The command line should follow the format \"func op1, op2\" or \"func op1\" or \"func\", but more words were found after the second operand", lineNum);
    
}

/* if we've reaches a data declaration line, add all the numbers to our memory */
void addData(char currLine[]) {
    char currWord[MAXLENGTHLINE];
    int data, lineLength = (int) strlen(currLine);
    goNextWord(NULL, currWord);
    char *position = strstr(currLine, currWord);
    int startingDC = DC;

    int index = (int) (position - currLine);

    int start, end, didFindComa = 1;

    while (index < lineLength && currLine[index] != '\n') {
        if (!didFindComa){
            raiseError("Expected comma-separated numbers after the .data declaration, but received unexpected input.", lineNum);
            return;
        }
        if (!isNumber(currWord)){
            raiseError("Expected comma-separated numbers after the .data declaration, but received unexpected input.", lineNum);
            return;
        }
        data = atoi(currWord);
        addDataToList(data);
        DC++;

        /* reach next number, also check that we indeed encountered a comma between the numbers */
        didFindComa = 0;
        while (isDigit(currLine[index])){
            index++;
        }
        while (index < lineLength && !isDigit(currLine[index]) && currLine[index] != '-'){
            if (currLine[index] == ','){
                if (didFindComa)
                    raiseError("Expected comma-separated numbers after the .data declaration, but received more than a singular comma between two numbers.", lineNum);
                didFindComa = 1;
            }

            index++;
        }
        if (index >= lineLength && didFindComa){
            raiseError("Expected comma-separated numbers after the .data declaration, but received an extra comma st the end.", lineNum);
        }

        /* this part gets the next number */
        start = index;
        if (currLine[index] == '-')
            index++;
        while (index < lineLength && isDigit(currLine[index])){
            index++;
        }
        if (index < lineLength && currLine[index] != ' ' && currLine[index] != ','){
            raiseError("Expected comma-separated numbers after the .data declaration, but received unexpected input.", lineNum);
            return;
        }
        index--;
        end = index;
        strcpyPart(currWord, currLine, start, end);
    }

    if (startingDC == DC)
        raiseWarning("this line is a data declaration but no numbers were given", lineNum);
}

/* return 1 if this line has a label declaration and 0 otherwise */
int isLineLabelDec(char currLine[]) {
    char currWord[MAXLENGTHLINE];
    getWord(currLine, 0, currWord);
    if (currWord[strlen(currWord) - 1] == ':')
        return 1;
    return 0;
}

/* return 1 if this line is a "storing data" line and 0 otherwise */
int isLineDataDec(char currLine[]) {
    char *pos;
    int indexOfWord;
    pos = strstr(currLine, ".data");
    if (pos) {
        indexOfWord = (int) (pos - currLine);
        if ((indexOfWord != 0 && currLine[indexOfWord - 1] != ' ') || (indexOfWord + 5 < strlen(currLine) && currLine[indexOfWord + 5] != ' '))
            raiseError("The use of '.' within a word is prohibited. Did you intend to use \".data\" instead?", lineNum);
        else
            return 1;
    }

    pos = strstr(currLine, ".string");
    if (pos) {
        indexOfWord = (int) (pos - currLine);
        if ((indexOfWord != 0 && currLine[indexOfWord - 1] != ' ') || (indexOfWord + 7 < strlen(currLine) && currLine[indexOfWord + 7] != ' '))
            raiseError("The use of '.' within a word is prohibited. Did you intend to use \".data\" instead?", lineNum);
        else
            return 1;
    }

    return 0;
}


/* if this word is already existing label, return it, else, return NULL */
Label *isLabel(char currWord[]) {
    Label *pLabel = lastLabel;
    while (pLabel) {
        if (customStrCmp(currWord, pLabel->name) == 0) {
            return pLabel;
        }
        pLabel = pLabel->next;
    }
    return NULL;  /* no label definition found */
}

/* this function receives a word and removes the colon at the end of it (if there's one) */
void removeColon(char currWord[]) {
    int lastChar = (int) (strlen(currWord) - 1);
    if (currWord[lastChar] == ':')
        currWord[lastChar] = '\0';
}

/* adds a new label's definition to our collection */
int addDataLabel(char *name) {
    /* check if the label's name is valid */
    if (!isValidLabelName(name)){
        raiseError("a label's name can only contain digits and letters (both uppercase and lowercase)", lineNum);
    }
    /* check if the label's name already belongs to another angle */
    if (isLabel(name)) {
        raiseError("Label name already assigned in this file", lineNum);
    }
    if (isRealFunc(name))
	raiseError("Label name isn't valid", lineNum);
    Label *newLabel = (Label *) malloc(sizeof(Label));
    if (!newLabel) {
        raiseError("problem regarding memory allocation of a Label", lineNum);
        return 0;
    }
    newLabel->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
    if (!newLabel->name) {
        raiseError("memory allocation of the Label's name failed", lineNum);
        free(newLabel);
        return 0;
    }
    removeColon(name);
    strcpy((newLabel->name), name);
    newLabel->line = DC;
    newLabel->type = 'd';
    newLabel->isEntry = 0;
    newLabel->next = lastLabel;
    lastLabel = newLabel;
    return 1;
}

/* returns 1 if the label's name is valid and 0 otherwise */
int isValidLabelName (char labelName[]){
    const char validCharacters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char temp[2];
    int i;

    for (i = 0; i < strlen(labelName) - 1; ++i) {
        sprintf(temp, "%c", labelName[i]);
        if (strstr(validCharacters, temp) == NULL)
            return 0;
    }
    return 1;
}

/* adds a new label's definition to our collection */
int addFuncLabel(char *name) {
    removeColon(name);
    if (!isValidLabelName(name)){
        raiseError("a label's name can only contain digits and letters (both uppercase and lowercase)", lineNum);
    }
    if (isRealFunc(name))
	raiseError("Label name can't be the same as an existing assembly keyword", lineNum);
    if (isLabel(name)) {
        raiseError("Label name already assigned in this file", lineNum);
    }
    Label *newLabel = (Label *) malloc(sizeof(Label));
    if (!newLabel) {
        raiseError("problem regarding memory allocation of a Label", lineNum);
        return 0;
    }
    newLabel->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
    if (!newLabel->name) {
        raiseError("memory allocation of the Label's name failed", lineNum);
        free(newLabel);
        return 0;
    }
    strcpy((newLabel->name), name);
    newLabel->line = IC;
    newLabel->type = 'c';
    newLabel->isEntry = 0;
    newLabel->next = lastLabel;
    lastLabel = newLabel;
    return 1;
}

/* adds a new Data's definition to our collection */
void addDataToList(int data) {
    /* create a new "data" in the memory and check that it was created properly */
    dataTable *newData = (dataTable *) malloc(sizeof(dataTable));
    if (!newData) {
        raiseError("problem regarding memory allocation of a Data", lineNum);
        return;
    }
    newData->data = data;
    newData->next = lastData;
    lastData = newData;
}

/* like strcpy but for a part of a string */
void strcpyPart(char *destination, const char *origin, int start, int end) {
    int length = end - start + 1;

    if (end >= strlen(origin))
        return;

    /* Copy the substring to the destination */
    strncpy(destination, origin + start, length);

    destination[length] = '\0';
}

/* adds a new Data's definition to our collection */
void addStringToList(char currLine[], int indexOfString) {
    int i = indexOfString;
    int lengthLine = (int) strlen(currLine);
    dataTable *newData;

    if (currLine[indexOfString] != '"'){
        raiseError("After encountering '.string', a valid input is expected in the form of a string enclosed within double quotation marks (\"...\") but the first \" is missing.", lineNum);
        return;
    }

    /* go to the index of the first character in the given string */
    i++;

    while (i < lengthLine && currLine[i] != '"') {

        /* create a new "data" in the memory and check that it was created properly */
        newData = (dataTable *) malloc(sizeof(dataTable));
        if (!newData) {
            raiseError("problem regarding memory allocation of a Data", lineNum);
            return;
        }
        newData->data = (int) currLine[i];
        newData->next = lastData;
        lastData = newData;
        DC++;
        i++;
    }

    /* finally, add '\0' to the memory */
    newData = (dataTable *) malloc(sizeof(dataTable));
    /* create a new "data" in the memory and check that it was created properly */
    if (!newData) {
        raiseError("problem regarding memory allocation of a Data", lineNum);
        return;
    }
    DC++;
    newData->data = (int) '\0';
    newData->next = lastData;
    lastData = newData;

    /* check if we reached the end of the string, that's a problem because we still haven;t reached the second " */
    if (i >= lengthLine){
        raiseError("After encountering '.string', a valid input is expected in the form of a string enclosed within double quotation marks (\"...\") but the second \" is missing.", lineNum);
        return;
    }

    /* check if there's more words in this line */
    i = nextArg(currLine, i + 1);
    if (currLine[i] != '\n' && i != -1)
        raiseError("After encountering '.string', a valid input is expected in the form of a string enclosed within double quotation marks (\"...\") but There are additional elements following the second \".", lineNum);
}

/* we've already checked this line is a "storing data" line, we'll add that data here whether it's integers or a string */
void addDataAndString(char currLine[], char currWord[]) {
    if (strcmp(currWord, ".data") == 0)
        addData(currLine);
    else{
        goNextWord(NULL, currWord);
        char *pos = strstr(currLine, currWord);
        int indexOfString = (int) (pos - currLine);
        addStringToList(currLine, indexOfString);
    }
}

/* returns 1 if this line is an extern declaration, -1 if this line is an entry declaration and 0 otherwise */
int entryOrExtern(char currLine[], char currWord[]) {
    goNextWord (currLine, currWord);
    if (strcmp(currWord, ".extern") == 0)
        return 1;
    if (strcmp(currWord, ".entry") == 0)
	return -1;
    goNextWord(NULL, currWord);
    if (strcmp(currWord, ".extern") == 0)
        return 1;
    if (strcmp(currWord, ".entry") == 0)
	return -1;
    return 0;
}

/* returns 1 if this line is an extern declaration and 0 otherwise */
int isExtern(char currLine[]) {
    char currWord[MAXLENGTHLINE];
    getWord(currLine, 0, currWord);
    if (strstr(currWord, ".extern"))
        return 1;
    return 0;
}

/* add all the labels defined to be extern in this line and mark them (newLabel->type = 'e') */
void addExternals(char currLine[], int index) {
    char name[MAXLENGTHLINE];
    Label *newLabel;
    while (index < strlen(currLine)) {
        getWord(currLine, index, name);

        /* check if this label already exists */
        if (isLabel(name)) {
            raiseError("Label name already assigned in this file", lineNum);
        }
	if (isRealFunc(name) || !isValidLabelName(name))
	    raiseError("Label name isn't valid", lineNum);

        /* create a new label in the memory and check that it was created properly */
        newLabel = (Label *) malloc(sizeof(Label));
        if (!newLabel) {
            raiseError("problem regarding memory allocation of a Label", lineNum);
            return;
        }
        /* create an array to sore the label's name and check that it was created properly */
        newLabel->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
        if (!newLabel->name) {
            raiseError("memory allocation of the Label's name failed", lineNum);
            free(newLabel);
            return;
        }
        strcpy((newLabel->name), name);
        newLabel->line = -1;
        newLabel->type = 'e';
        newLabel->isEntry = 0;
        newLabel->next = lastLabel;
        lastLabel = newLabel;

        /* "index" moves to the next word */
        index += (int) strlen(name) + 3;
    }
}

/* returns 1 if this word is an existing word in assembly and 0 otherwise */
int isRealFunc(char currWord[]) {
    int i;
    const char *funcNames[] = {
            "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
            "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
    };
    for (i = 0; i < (sizeof(funcNames) / sizeof(funcNames[0])); i++) {
        if (strcmp(currWord, funcNames[i]) == 0)
            return 1;
    }
    
    return 0;
}

/* go to the next word in the line */
void goNextWord(const char *currLine, char *currWord) {
    static const char *position = NULL; /* Position tracker in the line */
    
    /* If a new line is provided, update the position */
    if (currLine != NULL) {
        position = currLine;
    }
    
    int wordLength = 0;

    /* Copy characters until whitespace or end of string */
    while (*position != '\0' && *position != '\n' && *position != ' ') {
        currWord[wordLength] = *position;
        wordLength++;
        position++;
    }

    /* Null-terminate the copied word */
    currWord[wordLength] = '\0';

    /* Move position to the next non-whitespace character (the next word) */
    position = skipWhitespace(position);
}

/* skip whitespace characters */
const char *skipWhitespace(const char *str) {
    while (*str == ' ') {
        str++;
    }
    return str;
}

/* compute the amount of machine code lines this line creates */
int computeL(char func[], const char op1[], const char op2[]) {
    int L;
    /* after these if else statements L will be the number of operands this function uses */

    /* functions that receive 2 operands */
    if (strcmp(func, "mov") == 0 || strcmp(func, "cmp") == 0 || strcmp(func, "add") == 0 ||
        strcmp(func, "sub") == 0 || strcmp(func, "lea") == 0) {
        if (op1[0] == '\0' || op2[0] == '\0')
            raiseError("The function on this line expects two arguments, but fewer were provided", lineNum);
        L = 3;

        /* functions that receive 1 operand */
    } else if (strcmp(func, "jmp") == 0 || strcmp(func, "bne") == 0 || strcmp(func, "red") == 0 ||
               strcmp(func, "prn") == 0 || strcmp(func, "jsr") == 0 || strcmp(func, "not") == 0 ||
               strcmp(func, "clr") == 0 || strcmp(func, "inc") == 0 || strcmp(func, "dec") == 0) {
        if (op1[0] == '\0' || op2[0] != '\0')
            raiseError("The function on this line expects one argument, but a different amount were provided", lineNum);
        L = 2;

        /* functions that receive 0 operands */
    } else if (strcmp(func, "rts") == 0 || strcmp(func, "stop") == 0){
        if (op1[0] != '\0')
            raiseError("The function on this line expects no arguments, but some were provided", lineNum);
        L = 1;
    }
    else
        raiseError("unknown function", lineNum);


    /* the only way the amount of machine code lines this line creates is different from the number of operands this function gets + 1 is when both operands are registers, in that case the binary code is stored on the same line */
    if (op1[0] == '@' && op2[0] == '@')
        return L - 1;

    return L;
}

/* returns the binary string of this function */
char *binaryOfFunc(char func[]) {
    if (strcmp(func, "mov") == 0) {
        return "0000";
    } else if (strcmp(func, "cmp") == 0) {
        return "0001";
    } else if (strcmp(func, "add") == 0) {
        return "0010";
    } else if (strcmp(func, "sub") == 0) {
        return "0011";
    } else if (strcmp(func, "not") == 0) {
        return "0100";
    } else if (strcmp(func, "clr") == 0) {
        return "0101";
    } else if (strcmp(func, "lea") == 0) {
        return "0110";
    } else if (strcmp(func, "inc") == 0) {
        return "0111";
    } else if (strcmp(func, "dec") == 0) {
        return "1000";
    } else if (strcmp(func, "jmp") == 0) {
        return "1001";
    } else if (strcmp(func, "bne") == 0) {
        return "1010";
    } else if (strcmp(func, "red") == 0) {
        return "1011";
    } else if (strcmp(func, "prn") == 0) {
        return "1100";
    } else if (strcmp(func, "jsr") == 0) {
        return "1101";
    } else if (strcmp(func, "rts") == 0) {
        return "1110";
    } else if (strcmp(func, "stop") == 0) {
        return "1111";
    }
    return NULL;
}

/* returns 1 if this char is a digit and 0 otherwise */
int isDigit (char c){
    if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9')
        return 1;
    return 0;
}

/* returns 1 if this string is an integer and 0 otherwise */
int isNumber(char str[]) {
    int i = 0;
    if (str[0] == '\0')
        return 0;

    /* if there's a minus sign ignore it */
    if (str[0] == '-')
        i++;

    for (; str[i] != '\0'; i++) {
        if (!isDigit(str[i])) {
            return 0;
        }
    }
    return 1;
}
/* returns the code of this operand according to the table on page 23 of the course's textbook */
char *opCode(char op[]) {
    /* check if the operator is a number */
    if (isNumber(op))
        return "001";
    else if (op[0] == '@')
        return "101";
    else if (op[0] == '\0')
        return "000";
    else
        return "011";
}

/* for every label that points to a "data" in the memory add IC */
void addICToData() {
    Label *pLabel = lastLabel;
    while (pLabel) {
        if (pLabel->type == 'd')
            pLabel->line += IC;
        pLabel = pLabel->next;
    }
}
