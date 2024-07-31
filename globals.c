#include "globals.h"

#include <stdio.h>

/*
 *  this file is responsible for creating and initializing the global variables in this program
 */

int lineNum = 0;
int isErrorHappened = 0;
int IC = 100;
int DC = 0;
Label *lastLabel = NULL;
dataTable *lastData = NULL;
