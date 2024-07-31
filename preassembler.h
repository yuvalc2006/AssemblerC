#ifndef ASSEMBLERYUVAL_FIRSTSWEEP_H
#define ASSEMBLERYUVAL_FIRSTSWEEP_H

typedef struct Macro Macro;

/* A struct that contains the name and the definition of a macro*/
struct Macro {
    char *name;
    char *text;
    Macro *next;
};

void addMacro(char *name, char *text);

int preAsm(char fileName[]);

int didntReachEndMacro(char currLine[]);

int checkMacroDef(char currLine[], char macroName[]);

int isMacroDef(char *currLine);

void ignoreMacro(char *currLine, FILE *assFile);

void replaceMacros(char currLine[]);

char *isMacro(char currWord[]);

void separateNewLine (char currLine[]);

void addWordWithCommas(char *line, char *word);

#endif
