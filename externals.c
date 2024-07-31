#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include "firstsweep.h"
#include "main.h"
#include "constants.h"
#include "globals.h"
#include "externals.h"

Extern *lastExtern = NULL;

int computerCodeLineNum = 100;

int isFirst = 1;

void createExt (char fileName[]){
    replaceExtension(fileName, "ext");

    FILE *extFile = fopen(fileName, "w");
    if (!extFile) {
        raiseError("Could not create externals file.", 0);
        return;
    }

    replaceExtension(fileName, "txt");

    FILE *assFile = fopen(fileName, "r");
    if (!assFile) {
        raiseError("Could not read from preassembler file.", 0);
        return;
    }
    lineNum = 0;
    IC = 0;
    DC = 0;

    Label *pLabel = lastLabel;

    /* find all the externals in this file */
    while (pLabel){
        if (pLabel->type == 'e')
            addExtern(pLabel);

        pLabel = pLabel->next;
    }

    /* if there aren't any externals in this file, return */
    if (lastExtern == NULL){
        fclose(extFile);
        fclose(assFile);

        removeFile(fileName, "ext");

	return;
    }

    char currLine[MAXLENGTHLINE], currWord[MAXLENGTHLINE], func[5], op1[MAXLENGTHLINE], op2[MAXLENGTHLINE];
    int index;


    /* go through every line and if it's a command line, search for the extern labels */
    while (nextLine(currLine, assFile)){
	index = nextArg(currLine, 0);
	if (currLine[index] == ';' || currLine[index] == '\n' || currLine[index] == '\0'){
            continue;
	}	

        if (isCommandLine(currLine)){

            goNextWord(currLine, currWord);

            if (isLineLabelDec(currLine))
                goNextWord(NULL, currWord);

	    /* get all of the words in this command line and if one of them is an extern label, we'll print it to file */

            getFuncAndOps(currLine, currWord, func, op1, op2);

            printExternalsInLine(extFile, op1, op2);

            computerCodeLineNum += computeL(func, op1, op2);
        }
    }

    freeExternals();

    fclose(assFile);
    fclose(extFile);
}

/* free all of the extern structs once we're finished */
void freeExternals(){
    Extern *pExtern = lastExtern;
    Extern *tempExtern;

    while (pExtern){
	tempExtern = pExtern;
	pExtern = pExtern->next;
        /* free the memory for the extern structure itself */
        free(tempExtern);
    }
}

/* print the external labels in this line */
void printExternalsInLine (FILE *extFile, char op1[], char op2[]){
    char printLine[MAXLENGTHLINE];
    Extern *pExtern = lastExtern;
    /* for each extern label we've found, we'll search if it appears in this line and if it does, print it */
    while (pExtern){
        if (strcmp(op1, pExtern->label.name) == 0){
            sprintf(printLine, "%s %d", pExtern->label.name, computerCodeLineNum + 1);
            printExtToFile(extFile, printLine);
            if (isFirst)
                isFirst = 0;
        }

        if (strcmp(op2, pExtern->label.name) == 0){
            sprintf(printLine, "%s %d", pExtern->label.name, computerCodeLineNum + 2);
            printExtToFile(extFile, printLine);
            if (isFirst)
                isFirst = 0;
        }

        pExtern = pExtern->next;
    }
}


void printExtToFile (FILE *extFile, char line[]){
    if (!isErrorHappened){
        if (!isFirst)
            fprintf(extFile, "\n%s", line);
        else
            fprintf(extFile, "%s", line);
    }
}

/* create a node in the struct Extern */
void addExtern (Label *label){
    Extern *newExtern = (Extern *) malloc(sizeof(Extern));
    if (!newExtern) {
        raiseError("problem regarding memory allocation of a Data", lineNum);
        return;
    }
    newExtern->label = *label;
    newExtern->next = lastExtern;
    lastExtern = newExtern;
}

/* return 1 if this line is a command line, return 0 otherwise */
int isCommandLine (char currLine[]){
    int index = nextArg(currLine, 0);
    char currWord[MAXLENGTHLINE];
    /* a line has to be an empty line, a comment line, a storing data line or a commend line */
    if (currLine[index] == '\0' || currLine[index] == '\n' || currLine[index] == ';' || isLineDataDec(currLine) || entryOrExtern(currLine, currWord))
        return 0;
    return 1;
}
