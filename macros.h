#ifndef MACROS_H
#define MACROS_H

typedef struct Macro
{
    char name[MAX_MACRO_LEN]; /* contains the macro's name */
    char* content; /* the macro's body */
} Macro;

void initMacro(Macro * macro);
void addMacro(Macro newMacro);
void readMacro(FILE *fptr, char *name, int *lineNum);
int checkMacroPlacement(char* line);
void freeMacroTable();

#endif