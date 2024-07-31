#include<stdio.h>
#include<string.h>
#include <malloc.h>

#include "preassembler.h"
#include "main.h"
#include "constants.h"
#include "globals.h"

/*
 * This file is responsible for the entire "pre assembler" process of replacing macros
 */
Macro *firstMacro = NULL;

int preAsm(char fileName[]) {
    lineNum = 0;
    char currLine[MAXLENGTHLINE], macroName[MAXLENGTHLINE], macroText[MAXLENGTHFILE] = "";

    replaceExtension(fileName, "as");
    FILE *assFile = fopen(fileName, "r");
    if (!assFile) {
        raiseErrorInFile("Could not open the input \".as\" file.", fileName);
        return 1;
    }

    replaceExtension(fileName, "txt");
    FILE *newFile = fopen(fileName, "w");
    if (!newFile) {
        raiseErrorInFile("Could not open \"pre assembler\" file.", fileName);
        fclose(assFile);  /* Close the previously opened file */
        return 1;
    }

    int isMacroDefinition, isMacroDefGood;

    /* Process each line in the input assembly file */
    while (nextLine(currLine, assFile)) {
        /* Replace macros in the current line with their definitions */
        replaceMacros(currLine);

        /* Check if the current line is a macro definition */
        isMacroDefinition = isMacroDef(currLine);

        if (isMacroDefinition) {

            /* Extract macro name (it's in index 5 because of replaceMacros organisation */
            getWord(currLine, 5, macroName);

            /* check the macros' name, if it's problematic we'll ignore this macro */
            isMacroDefGood = checkMacroDef(currLine, macroName);
            if (!isMacroDefGood) {
                ignoreMacro(currLine, assFile);
                continue;
            }

            /* prepare to get the macro's definition by going to the next line and clear the macroText and the macroName buffers for each new macro definition */
            nextLine(currLine, assFile);
            macroText[0] = '\0';
            macroName[strlen(macroName) - 1] = '\0';

            /* Collect macro definition until "endmcro" is reached */
            while (didntReachEndMacro(currLine)) {
                /* before copying this line we'll search for macros and replace them */
                replaceMacros(currLine);

                /* copy this line */
                strcat(macroText, currLine);
                nextLine(currLine, assFile);
            }
            /* Add macro to collection */
            addMacro(macroName, macroText);

            /* we've finished reading the macros end we're currently at the "endmcro" line, we'll start the loop again to keep running the code for the next line */
            continue;

        }
        /* We need to check that we didn't reach "endmcro" (that's a problem because we're not in a macro declaration) */
        if (!didntReachEndMacro(currLine)) {
            raiseError("reached \"endmcro\" without a macro declaration beforehand", lineNum);
            continue;
        }


        fprintf(newFile, "%s", currLine); /* Write the modified line to the output file */
    }

    /* Free memory for macros */
    while (firstMacro) {
        Macro *temp = firstMacro;
        firstMacro = firstMacro->next;
        free(temp->name);
        free(temp->text);
        free(temp);
    }

    fclose(assFile);
    fclose(newFile);
    return 0;
}

/* in some cases, the input is so problematic we need to ignore the macro's definition (for example: macro doesn't have a name), this function's job is to ignore the macro's declaration */
void ignoreMacro(char *currLine, FILE *assFile) {
    while (didntReachEndMacro(currLine)) { /* we go through every line until we reach 'endmcro' */
        nextLine(currLine, assFile);
    }
}

/* replaces all the macros in the line with their definitions */
void addWordWithCommas(char *line, char *word) {
    if (line == NULL || word == NULL) {
	/* Handle invalid input */
        return; 
    }

    if (strlen(word) == 1 && word[0] == ','){
        strcat(line, word);
        return;
    }

    /* Check if the word needs to be separated by commas */
    int leadingComma = (word[0] == ',');
    int trailingComma = (word[strlen(word) - 1] == ',');

    /* Append the word to the line with proper comma separation */
    if (leadingComma && trailingComma) {
        /* handle the case (,a, -> , a ,) */
        strcat(line, ", ");
        strncat(line, word + 1, strlen(word) - 2);
        strcat(line, " ,");
    } else if (leadingComma) {
        /* handle the case (,a -> , a) */
        strcat(line, ", ");
        strcat(line, word + 1);
    } else if (trailingComma) {
        /* handle the case (a, -> a ,) */
        strncat(line, word, strlen(word) - 1);
        strcat(line, " ,");
    } else {
        /* lastly, handle the case (a -> a) */
        strcat(line, word);
    }
}

/* replaces all the macros in the line with their definitions */
void replaceMacros(char currLine[]) {
    /* Create a new line for modified output with double the length to account for the spaces we're going to add */
    char newLine[MAXLENGTHLINE * 2] = "";
    char currWord[MAXLENGTHLINE];
    int i = nextArg(currLine, 0);

    if (i == -1){
        currLine[0] = '\n';
        currLine[1] = '\0';
    }

    /* Iterate through each word in the current line */
    while (i != -1) {
        getWord(currLine, i, currWord);
        char *macroDef = isMacro(currWord);
        if (macroDef != NULL) {
            /* Append macro definition to newLine */
            strcat(newLine, macroDef);
        } else {
            addWordWithCommas(newLine, currWord);
        }
        /* Add a space separator */
        strcat(newLine, " ");
        i = nextArg(currLine, i + (int) strlen(currWord));
    }
    separateNewLine(newLine);


    /* copy the modified line back to currLine */
    strcpy(currLine, newLine);
}



/* replaceMacros has a problem with '\n' because it treats them as a part of a word, this function helps it */
void separateNewLine (char currLine[]){
    int length = (int) (strlen(currLine) - 1);
    /* the while loop in replaceMacros will add a space at the end, so we need to check before that space, we'll check in length - 1 */
    if (currLine[length - 1] == '\n'){
        currLine[length - 1] = ' ';
        currLine[length] = '\n';
    }

    /* do the same for '\0' */
    if (currLine[length - 1] == '\0'){
        currLine[length - 1] = ' ';
        currLine[length] = '\0';
    }
}

/* if this word is a macro we found, return his definition, if not, return NULL */
char *isMacro(char currWord[]) {
    Macro *pMacro = firstMacro;
    while (pMacro) {
        if (customStrCmp(currWord, pMacro->name) == 0) {
	    /* Return macro definition */
            return pMacro->text;  
        }
        pMacro = pMacro->next;
    }
    return NULL;  /* no macro definition found */
}


/* returns 0 if we've reached 'endmcro' and 1 if not */
int didntReachEndMacro(char currLine[]) {
    char *t1 = strstr(currLine, "endmcro ");
    char *t2 = strstr(currLine, "endmcro\n");
    char *t3 = strstr(currLine, "endmcro\t");
    if (t1 || t2 || t3) {
        /* if the length is more than just 7 or 8 (length of endmcro with and without \n) theres more words in the line */
        if (strlen(currLine) > 8)
            raiseError("An isolated \"endmcro\" is expected", lineNum);
        return 0;
    }
    return 1;
}

/* adds a new macro's definition to our collection */
void addMacro(char *name, char *text) {
    if (isMacro(name)) {
        raiseError("macro name already assigned", lineNum);
    }
    Macro *newMacro = (Macro *) malloc(sizeof(Macro));
    if (!newMacro) {
        raiseError("problem regarding memory allocation of a macro", lineNum);
        return;
    }
    newMacro->name = (char *) malloc((strlen(name) + 1) * sizeof(char));
    if (!newMacro->name) {
        raiseError("memory allocation of the macro's name failed", lineNum);
        free(newMacro);
        return;
    }
    strcpy((newMacro->name), name);
    newMacro->text = (char *) malloc((strlen(text) + 1) * sizeof(char));
    if (!newMacro->text) {
        raiseError("memory allocation of the macro's definition failed", lineNum);
        free(newMacro);
        return;
    }
    strcpy((newMacro->text), text);
    newMacro->next = firstMacro;
    firstMacro = newMacro;
}

/* Returns 0 if the macro's name is illegal (name already exists in assembly) and 1 if it's fine */
int checkMacroDef(char currLine[], char macroName[]) {
    int i;
    int indexOfNextWord = nextArg(currLine, (int) (5 + strlen(macroName)));
    if (currLine[indexOfNextWord] != '\n' && currLine[indexOfNextWord] != '\0') {
        raiseError("Additional words are not allowed after the macro's name", lineNum);
        return 0;
    }

    const char *funcNames[] = {
            "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec",
            "jmp", "bne", "red", "prn", "jsr", "rts", "stop", ".data", ".string", ".entry", ".extern"
    };

    /* Check if the name is present in the funcNames array */
    for (i = 0; i < sizeof(funcNames) / sizeof(funcNames[0]); i++) {
        if (customStrCmp(macroName, funcNames[i]) == 0) {
            raiseError("the macro's name cannot be the same as an existing function", lineNum);
            return 0; /* The name is not a valid function name */
        }

    }
    /* The name is a valid function name, return 1 */
    return 1;
}

/* This function checks for a macro definition in the given current line.
   It returns:
   - 0 if there's no macro definition or if there's a problem in the definition
   - 1 if everything is correct */
int isMacroDef(char *currLine) {
    /* Search for the occurrence of "mcro" at the beginning of the line */
    if (strstr(currLine, "mcro ") == currLine) {
        /* The line starts with "mcro ", indicating a valid macro definition */
        char macroName[MAXLENGTHLINE];
        getWord(currLine, 5, macroName);
        if (checkMacroDef(currLine, macroName)) {
            return 1;
        } else {
            return 0;
        }
    } else if (strstr(currLine, "mcro") == currLine) {
        /* "mcro" is at the beginning of the line, but no space follows it so no name was given */
        raiseError("Expecting name after \"mcro\"", lineNum);
        return 0;
    } else if (strstr(currLine, " mcro ") != NULL || strstr(currLine, " mcro") != NULL) {
        /* "mcro" is found, but not at the beginning of the line */
        raiseError("An isolated \"mcro\" is expected", lineNum);
        return 0;
    }

    /* No macro definition found in the current line */
    return 0;
}
