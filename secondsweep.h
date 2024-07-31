#ifndef SECONDSWEEP_H
#define SECONDSWEEP_H

#include <stdio.h>

#define charToInt(c) (int)(c - '0')

int secondSweep(char fileName[]);

void markEntry(char labelName[]);

void printOpCodes (FILE *newFile, char func[], char op1[], char op2[]);

void createEnt (char fileName[]);

void printFuncBinCode (FILE *newFile, char func[], char opCode1[], char opCode2[]);

char *intToBinaryString(int num, int length);

void printToFile (FILE *newFile, char newLine[]);

void printLabelOpCode (FILE *newFile, char op[], char newLine[]);

void printNumberOpCode (FILE *newFile, char op[], char newLine[]);

void printRegisterOpCode (FILE *newFile, char op[], char newLine[], int numOfOp);

void printBothRegistersOpCode (FILE *newFile, char op1[], char op2[], char newLine[]);

void checkOps (char func[], char opCode1[], char opCode2[]);

void printToEntFile (FILE *newFile, char newLine[], int count, int numEntries);

void checkRegisterName (char reg[]);

char base64EncodeChar(int num);

int binaryToDecimal(const char *binary, int start, int end);

void printInBase64 (FILE *newFile, char binCode[]);

void printDataInBase64(FILE *newFile, int data);

void printData(FILE *newFile);

#endif
