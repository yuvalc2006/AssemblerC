#ifndef MAIN_H
#define MAIN_H

void raiseError(char error[], int line);

int fileExists(const char *filename);

int isExistingFile (char fileName[]);

void raiseErrorInFile (char error[], char fileName[]);

void freeLabelAndData ();

void removeFile (char fileName[], char extension[]);

void deleteFiles (char fileName[]);

void raiseWarning(char warning[], int line);

void welcomeMessage();

int nextLine(char *currLine, FILE *assFile);

int getWord(char currLine[], int i, char word[]);

int customStrCmp(const char *str1, const char *str2);

int nextArg(char currLine[], int i);

void safeStrCpy(char *dest, const char *src, size_t destSize);

void replaceExtension(char *filename, const char *newExtension);

#endif
