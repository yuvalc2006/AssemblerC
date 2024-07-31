#ifndef EXTERNALS_H
#define EXTERNALS_H

#include <stdio.h>

#include "firstsweep.h"

typedef struct Extern Extern;

/* we've created a struct that holds all of the extern labels in this file as nodes */
struct Extern {
    Label label;
    Extern *next;
};

void createExt (char fileName[]);

void freeExternals();

void addExtern (Label *label);

int isCommandLine (char currLine[]);

void printExtToFile (FILE *extFile, char line[]);

void printExternalsInLine (FILE *extFile, char op1[], char op2[]);

#endif
