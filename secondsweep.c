#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "firstsweep.h"
#include "secondsweep.h"
#include "main.h"
#include "constants.h"
#include "globals.h"
#include "externals.h"

/*
 * this file is responsible for the "second pass" process of generating all the files
 */

int ICandDC;

int secondSweep(char fileName[]) {

    if (isErrorHappened)
        return 0;
    /* the steps I'll be referring to in this file's comments are the steps provided by the algorithm in page 36 of the course textbook */
    lineNum = 0;
    ICandDC = IC + DC;

    char currLine[MAXLENGTHLINE], currWord[MAXLENGTHLINE], func[4], op1[MAXLENGTHLINE], op2[MAXLENGTHLINE];
    replaceExtension(fileName, "txt");
    FILE *assFile = fopen(fileName, "r");
    if (!assFile) {
        raiseErrorInFile("Could not open \"pre assembler\" file.", fileName);
        return 0;
    }

    replaceExtension(fileName, "ob");
    FILE *newFile = fopen(fileName, "w");
    if (!newFile) {
	fclose(assFile);
        raiseErrorInFile("Could not open \".ob\" file.", fileName);
        fclose(assFile);  /* Close the previously opened file */
        return 0;
    }
    sprintf(currLine, "%d %d", IC - 100, DC);
    printToFile(newFile, currLine);
    int index;
    /* step 1 */
    IC = 100;

    /* Process each line in the input assembly file (step 2) */
    while (nextLine(currLine, assFile)) {
	index = nextArg(currLine, 0);

        if (currLine[index] == ';' || currLine[index] == '\n' || currLine[index] == '\0'){
            continue;
	}
        op1[0] = '\0';
        op2[0] = '\0';

        goNextWord(currLine, currWord);

        /* step 3 */
        if (isLineLabelDec(currLine))
            goNextWord(NULL, currWord);

        /* step 4 */
        if (isExtern(currLine) || isLineDataDec(currLine))
            continue;
	
        /* step 5 */
        if (strcmp(currWord, ".entry") == 0) {
            goNextWord(NULL, currWord);
            /* step 6 */
            markEntry(currWord);
            continue;
        }
	

        getFuncAndOps(currLine, currWord, func, op1, op2);

        /* step 7 and 8 */
        printOpCodes(newFile, func, op1, op2);

        /* we're in a loop so step 9 happens automatically */
    }

    printData(newFile);


    /* when we opened the ".ob" file we've already created it, so this part is erasing it if we had an error and stopping the creation of the other files, which is similar to step 10 */
    if (isErrorHappened){
	fclose(assFile);
	fclose(newFile);
        return 1;
    }


    /* create the entries file (if there are entries) */
    createEnt (fileName);
    /* create the externals file (if there are externals) */
    createExt (fileName);
    

    fclose(assFile);
    fclose(newFile);

    return 0;
}

/* create the entries file (if there are entries) */
void createEnt (char fileName[]){
    replaceExtension(fileName, "ent");

    FILE *entFile = fopen(fileName, "w");
    if (!entFile) {
        raiseError("Could not create entries file.", 0);
    }

    int countEntries = 0, count = 0;
    Label *pLabel = lastLabel;
    char newLine[MAXLENGTHLINE];

    while (pLabel){
        if (pLabel->isEntry)
            countEntries++;
        pLabel = pLabel->next;
    }

    if (countEntries == 0){
        remove(fileName);
        return;
    }

    pLabel = lastLabel;

    while (pLabel){
        if (pLabel->isEntry){
            count++;
            sprintf(newLine, "%s %d", pLabel->name, pLabel->line);
            printToEntFile(entFile, newLine, count, countEntries);
            newLine[0] = '\0';
        }
        pLabel = pLabel->next;
    }

    fclose(entFile);
}

/* print the given line in this file */
void printToEntFile (FILE *newFile, char newLine[], int count, int numEntries){
    if (!isErrorHappened){
        if (count != numEntries)
            fprintf(newFile, "%s\n", newLine);
        else
            fprintf(newFile, "%s", newLine);
    }

}

/* mark the label as entry */
void markEntry(char labelName[]) {
    Label *pLabel = isLabel(labelName);
    if (!pLabel) {
        raiseError(".entry should be called with an existing label", lineNum);
        return;
    }
    if (pLabel->type == 'e')
	raiseError("a label cannot be an extern and a entry", lineNum);
    pLabel->isEntry = 1;
}

/* receives two numbers (num, length) and returns a string that it's length is length and is the binary representation of num */
char *intToBinaryString(int num, int length) {
    int index;
    /* Create a string to hold the binary code */
    char *binaryString = (char *)malloc(sizeof(char) * (length + 1));

    if (binaryString == NULL) {
	raiseError("memory allocation a string failed", lineNum);
        return NULL;
    }

    /* Null-terminate the string */
    binaryString[length] = '\0';

    for (index = length - 1; index >= 0; index--) {
        binaryString[index] = (char)((num & 1) + '0');
        /* Shift the number right by one bit */
        num >>= 1; 
    }

    return binaryString;
}

/* print the machine code for the function */
void printFuncBinCode (FILE *newFile, char func[], char opCode1[], char opCode2[]){
    char binCode[13];
    strcpy(binCode, "");

    /* if there's only one operand we place him as the destination (bits 2-4) so we'll treat that as a special case */
    if (strcmp(opCode2, "000") == 0 && strcmp(opCode1, "000") != 0){
        strcpy(binCode, "000");
        strcat(binCode, binaryOfFunc(func));
        strcat(binCode, opCode1);
        strcat(binCode, "00");
    }
    else{
        /* crate the machine code from this function, if there's no operands it still works because opCode1, opCode2 are both 000 */
        strcpy(binCode, opCode1);
        strcat(binCode, binaryOfFunc(func));
        strcat(binCode, opCode2);
        strcat(binCode, "00");
    }

    binCode[12] = '\0';

    printInBase64(newFile, binCode);

}

/* makes sure that the register was written correctly */
void checkRegisterName (char reg[]){
    if (strlen(reg) != 3)
        raiseError("The first operand is expected to be a register as it begins with '@' and the correct format is '@r(num)', however the operand's length isn't 3 so it doesn't follow that format.", lineNum);

    if (reg[1] != 'r')
        raiseError("The first operand is expected to be a register as it begins with '@' and the correct format is '@r(num)', however 'r' is missing.", lineNum);

    if (!isdigit(reg[2]))
        raiseError("The first operand is expected to be a register as it begins with '@' and the correct format is '@r(num)', however no 'num' was provided.", lineNum);

    if (reg[2] == '8' || reg[2] == '9')
        raiseError("The first operand is expected to be an register, the correct format is '@r(num)' when num is a digit between 0-7, however 'num' is in between 8-9.", lineNum);
}

/* print the given line in this file */
void printToFile (FILE *newFile, char newLine[]){
    if (!isErrorHappened)
        fprintf(newFile, "%s\n", newLine);
}

/* Print the machine codes corresponding to this line */
void printOpCodes(FILE *newFile, char func[], char op1[], char op2[]) {
    char opCode1[4], opCode2[4], newLine[13];

    safeStrCpy(opCode1, opCode(op1), 4);
    safeStrCpy(opCode2, opCode(op2), 4);

    checkOps (func, opCode1, opCode2);

    /* print the first machine code line */
    printFuncBinCode(newFile, func, opCode1, opCode2);

    /* if both operands are registers print them in a single line */
    if (strcmp(opCode1, "101") == 0 && strcmp(opCode2, "101") == 0){
        printBothRegistersOpCode(newFile, op1, op2, newLine);
        return;
    }

    /* print binary code of first operand */


    /* if there's no first operand this is a function with no operands and we'll stop here */
    if (strcmp(opCode1, "000") == 0)
        return;

    /* if the first operand is a number print it in this form "(binary representation of number with length 10)00" */
    if (strcmp(opCode1, "001") == 0) {
        printNumberOpCode(newFile, op1, newLine);
    }

        /* if the first operand is a number print it in this form "(binary representation of the register's index with length 5)0000000" */
    else if (strcmp(opCode1, "101") == 0){
        printRegisterOpCode(newFile, op1, newLine, 1);
    }
        /* if we reached this point this should be a label */
    else {
        printLabelOpCode(newFile, op1, newLine);
    }
    /* print binary code of second operand */

    newLine[0] = '\0';

    /* if there's no second operand this is a function with 1 operand and we'll stop here */
    if (strcmp(opCode2, "000") == 0)
        return;

    /* if the second operand is a number print it in this form "(binary representation of number with length 10)00" */
    if (strcmp(opCode2, "001") == 0) {
        printNumberOpCode(newFile, op2, newLine);
    }
    else if (strcmp(opCode2, "101") == 0){
        printRegisterOpCode(newFile, op2, newLine, 2);
    }
    else {
        printLabelOpCode(newFile, op2, newLine);
    }
}

void checkOps (char func[], char opCode1[], char opCode2[]){
    /* we've already passed firstSweep that uses computeL, so we know the functions has the correct number of operands */
    /* now we'll make sure every function receives the right kind of operands */
    if (strcmp(func, "mov") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("mov's first operand cannot be a number", lineNum);
    } else if (strcmp(func, "cmp") == 0) {
        /* we'll do nothing because both operands can be anything in cmp */
    } else if (strcmp(func, "add") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("add's first operand cannot be a number", lineNum);
    } else if (strcmp(func, "sub") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("sub's first operand cannot be a number", lineNum);
    } else if (strcmp(func, "not") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("not's operand cannot be a number", lineNum);
    } else if (strcmp(func, "clr") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("clr's operand cannot be a number", lineNum);
    } else if (strcmp(func, "lea") == 0) {
        if (strcmp(opCode2, "011") != 0)
            raiseError("lea's second operand must be a label", lineNum);
    } else if (strcmp(func, "inc") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("inc's operand cannot be a number", lineNum);
    } else if (strcmp(func, "dec") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("dec's operand cannot be a number", lineNum);
    } else if (strcmp(func, "jmp") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("jmp's operand cannot be a number", lineNum);
    } else if (strcmp(func, "bne") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("bne's operand cannot be a number", lineNum);
    } else if (strcmp(func, "red") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("red's operand cannot be a number", lineNum);
    } else if (strcmp(func, "prn") == 0) {
        /* we'll do nothing because the operand can be anything in prn */
    } else if (strcmp(func, "jsr") == 0) {
        if (strcmp(opCode1, "001") == 0)
            raiseError("jsr's operand cannot be a number", lineNum);
    } else if (strcmp(func, "rts") == 0) {
        /* no operands, ignore */
    } else if (strcmp(func, "stop") == 0) {
        /* no operands, ignore */
    }
}

void printBothRegistersOpCode(FILE *newFile, char op1[], char op2[], char newLine[]) {
    checkRegisterName(op1);
    checkRegisterName(op2);

    char *op1Binary = intToBinaryString(charToInt(op1[2]), 5);
    char *op2Binary = intToBinaryString(charToInt(op2[2]), 5);

    if (op1Binary && op2Binary) {
        strcpy(newLine, op1Binary);
        strcat(newLine, op2Binary);
        strcat(newLine, "00");
        printInBase64(newFile, newLine);
    } else {
        raiseError("Error in memory allocation of binary conversion", lineNum);
    }

    free(op1Binary);
    free(op2Binary);
}

void printRegisterOpCode (FILE *newFile, char op[], char newLine[], int numOfOp){
    char *binStr;
    if (numOfOp == 1){
        checkRegisterName(op);

	binStr = intToBinaryString(charToInt(op[2]), 5);
	if (binStr == NULL){
	    raiseError("Error in memory allocation of binary conversion", lineNum);
	    return;
	}

        strcpy(newLine, binStr);
        strcat(newLine, "0000000");

	free(binStr);
    }
    else if (numOfOp == 2){
        checkRegisterName(op);

	binStr = intToBinaryString(charToInt(op[2]), 5);
	if (binStr == NULL){
	    raiseError("Error in memory allocation of binary conversion", lineNum);
	    return;
	}

        strcpy(newLine, "00000");
        strcat(newLine, binStr);
        strcat(newLine, "00");

	free(binStr);
    }

    printInBase64(newFile, newLine);
}

void printNumberOpCode (FILE *newFile, char op[], char newLine[]){
    char *binStr;

    binStr = intToBinaryString(atoi(op), 10);
    if (binStr == NULL){
	    raiseError("Error in memory allocation of binary conversion", lineNum);
	    return;
    }

    strcpy(newLine, binStr);
    strcat(newLine, "00");

    free(binStr);

    printInBase64(newFile, newLine);
}

void printLabelOpCode (FILE *newFile, char op[], char newLine[]){
    Label *pLabel = isLabel(op);
    /* check if this is a label, if not then we know it's not a label, a number or a register, so we'll raise an error */
    if (!isLabel(op))
        raiseError("first operand isn't defined in assembly language and isn't a label in this file", lineNum);
    /* check if this label is extern, if it is use 01 (E) in the A,R,E sector */
    if (pLabel->type == 'e'){
        strcpy(newLine, "000000000001");
    }
    else{
        /* this is a non-extern label, print it in the form "(binary representation of the label's line with length 10)10" */
	char *binStr;

	binStr = intToBinaryString(pLabel->line, 10);
	if (binStr == NULL){
	    raiseError("Error in memory allocation of binary conversion", lineNum);
	    return;
	}

        strcpy(newLine, binStr);
        strcat(newLine, "10");
	free(binStr);
    }
    /* print this line */
    printInBase64(newFile, newLine);
}

void printInBase64 (FILE *newFile, char binCode[]){
    IC++;
    int binarySubstrInDecimal = binaryToDecimal(binCode, 0, 5);
    char charInBase64 = base64EncodeChar(binarySubstrInDecimal);

    fprintf(newFile, "%c", charInBase64);
    binarySubstrInDecimal = binaryToDecimal(binCode, 6, 11);

    charInBase64 = base64EncodeChar(binarySubstrInDecimal);
    fprintf(newFile, "%c", charInBase64);

    ICandDC--;

    if (ICandDC != 0)
        fprintf(newFile, "\n");
}

int binaryToDecimal(const char *binary, int start, int end) {
    int decimal = 0;
    int power = 0;
    int i;

    /* Iterate through the specified range of bits in reverse order */
    for (i = end; i >= start; i--) {
        if (binary[i] == '1') {
            decimal += (int)pow(2, power);
        }
        power++;
    }

    return decimal;
}

char base64EncodeChar(int num) {
    const char base64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (num >= 0 && num < 64)
        return base64Table[num];
    else
        return '\0';
}

void printData(FILE *newFile) {
    /* Count the number of nodes in the linked list "struct dataTable" */
    int i, count = 0;
    dataTable *current = lastData;
    while (current) {
        count++;
        current = current->next;
    }

    int dataArray[count];

    /* Populate the array with data values in reverse order */
    current = lastData;
    for (i = 0; i < count; i++) {
        dataArray[i] = current->data;
        current = current->next;
    }

    /* Iterate through the array in reverse order and print data values to the file */
    for (i = count - 1; i >= 0; i--) {
        printDataInBase64(newFile, dataArray[i]);
    }

}

void printDataInBase64(FILE *newFile, int data){
    char *binStr;

    binStr = intToBinaryString(data, 12);
    if (binStr == NULL){
	raiseError("Error in memory allocation of binary conversion", lineNum);
	return;
    }

    printInBase64(newFile, binStr);

    free(binStr);
}
