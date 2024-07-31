#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "preassembler.h"
#include "firstsweep.h"
#include "secondsweep.h"
#include "main.h"
#include "constants.h"
#include "globals.h"


int main(int argc, char *argv[]) {
    welcomeMessage();
    if (argc < 2) {
        raiseError("no files were provided as operands", 0);
        return 0;
    }
    int i;
    for (i = 1; i < argc; i++) {
        lineNum = 0;
        isErrorHappened = 0;
        IC = 0;
        DC = 0;
	lastLabel = NULL;
	lastData = NULL;

	
        /* Checks for the presence of a file with the same name but a different extension that could impact our program */
        if  (isExistingFile(argv[i]))
            continue;

	/* run all of the project but in the start of both sweeps, if an error was detected so far, we immediatelly return */
        preAsm(argv[i]);

        firstSweep(argv[i]);
	
        secondSweep(argv[i]);

        removeFile(argv[i], "txt");

        if (isErrorHappened){
            deleteFiles(argv[i]);
        }

        freeLabelAndData();

    }
    return 0;
}

void welcomeMessage()
{
    printf("\n\nassembler project - maman 14\n\n");
    printf("Author: Yuval Cohen\n");
}

void deleteFiles (char fileName[]){
    removeFile(fileName, "ob");
    removeFile(fileName, "ent");
    removeFile(fileName, "ext");
}

void removeFile (char fileName[], char extension[]){
    replaceExtension(fileName, extension);
    remove(fileName);
}

void freeLabelAndData (){
    Label *pLabel = lastLabel;
    Label *tempLabel;
    dataTable *pData = lastData;
    dataTable *tempData;

    /* free memory for the label linked list */
    while (pLabel){
        tempLabel = pLabel;
        pLabel=pLabel->next;
        /* free the memory for the label's name */
        free(tempLabel->name);
        /* free the memory for the label structure itself */
        free(tempLabel);
    }

    /* free memory for the dataTable linked list */
    while (pData){
        tempData = pData;
        pData=pData->next;
        /* free the memory for the dataTable structure itself */
        free(tempData);
    }
}

int isExistingFile (char fileName[]){

    replaceExtension(fileName, "as");
    if (!fileExists(fileName))
        raiseErrorInFile("file doesn't exist with extension .as", fileName);

    replaceExtension(fileName, "txt");
    if (fileExists(fileName))
        raiseErrorInFile("file already exists with extension .txt", fileName);

    replaceExtension(fileName, "ob");
    if (fileExists(fileName))
        raiseErrorInFile("file already exists with extension .ob", fileName);

    replaceExtension(fileName, "ent");
    if (fileExists(fileName))
        raiseErrorInFile("file already exists with extension .ent", fileName);

    replaceExtension(fileName, "ext");
    if (fileExists(fileName))
        raiseErrorInFile("file already exists with extension .ext", fileName);


    return isErrorHappened;
}
/* return 1 if this gile exists, otherwise reurn 0 */
int fileExists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void replaceExtension(char *filename, const char *newExtension) {
    char *dot = strrchr(filename, '.');
    if (dot) {
	/* Remove the current extension */
        *dot = '\0';  
    }
    strcat(filename, ".");
    /* Add the new extension */
    strcat(filename, newExtension);
}

void raiseErrorInFile (char error[], char fileName[]){
    isErrorHappened = 1;
    replaceExtension(fileName, "as");
    printf("\nError in the file %s: %s\n", fileName, error);
}

void raiseError(char error[], int line) {
    isErrorHappened = 1;
    printf("\nError in line %d: %s\n", line, error);
}

void raiseWarning(char warning[], int line) {
    printf("Warning in line %d: %s\n", line, warning);
}

/* word now holds the word that starts at index i and ends when we reach a space, a tab or the end of the line */
int getWord(char currLine[], int i, char word[]) {
    int j = i;
    while (j < strlen(currLine) && currLine[j] != ' ' && currLine[j] != '\t') {
        j++;
    }
    strncpy(word, currLine + i, j - i);
    word[j - i] = '\0';
    if (i == j)
        return 0;
    return 1;
}

/* returns the index of the next character that isn't a space or a tab from index i */
int nextArg(char currLine[], int i) {
    if (currLine == NULL) {
        raiseError("invalid input for the \"nextArg\" function", lineNum);
        return -1;
    }

    size_t lineLength = strlen(currLine);

    while ((i < lineLength) && (currLine[i] == ' ' || currLine[i] == '\t')) {
        i++;
    }

    if (i >= lineLength) {
        return -1;
    }
    return i;
}


/* currLine moves on to the next line, lineNum updates, returns 0 if we've reached the end of the file */
int nextLine(char *currLine, FILE *currFile) {
    if (!currLine || !currFile) {
	/* Invalid parameters, raise an error */
	raiseError("invalid parameters while calling the \"nextFile\" function", lineNum);
        return 0;  
    }

    lineNum++;
    if (fgets(currLine, MAXLENGTHLINE, currFile)) {
	/* Successful read, return 1 */
        return 1;  
    }

    if (feof(currFile)) {
	/* End of file reached, return 0 */
        return 0;  
    } else {
	
        /* An error occurred while reading, return 0 */
        raiseError("Error reading from file", lineNum);
        return 0;
    }
}


int customStrCmp(const char *str1, const char *str2) {
    /* Get the lengths of the strings without counting trailing newline characters */
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);


    /* Remove trailing newline characters from str1 */
    while (len1 > 0 && (str1[len1 - 1] == '\n' || str1[len1 - 1] == '\r')) {
        len1--;
    }

    /* Remove trailing newline characters from str2 */
    while (len2 > 0 && (str2[len2 - 1] == '\n' || str2[len2 - 1] == '\r')) {
        len2--;
    }

    /* Compare the modified strings */
    return strncmp(str1, str2, len1 > len2 ? len2 : len1);
}


void safeStrCpy(char *dest, const char *src, size_t destSize) {
    if (dest == NULL || src == NULL || destSize == 0) {
        dest = NULL;
	/* Return early if any operand is NULL or if destSize is zero, in a normal strcpy we would encounter an error, that's why this function is safer */
        return;
    }

    size_t srcLen = strlen(src);

    /* Ensure the source string length fits within the destination size */
    if (srcLen >= destSize) {
        srcLen = destSize - 1;
    }

    /* Copy the source string to the destination */
    strncpy(dest, src, srcLen);
    /* null-terminate */
    dest[srcLen] = '\0';
}
