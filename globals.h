#ifndef GLOBALS_H
#define GLOBALS_H

#include "firstsweep.h"

extern int lineNum; /* current line */
extern int isErrorHappened; /* have we encountered an error in the program */
extern int IC; /* instructions counter */
extern int DC; /* data counter */
extern Label *lastLabel; /* last node in the chain of labels */
extern dataTable *lastData; /* last node in the chain of datas */

#endif
