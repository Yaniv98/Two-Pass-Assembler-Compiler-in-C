#include <stdio.h>
#include <stdlib.h>

#include "master_header.h"

Macro* macroTable = NULL;
static int macroCount = 0;


/* initializes a macro instance */
void initMacro(Macro *macro)
{
    macro -> name[0] = 0;
    macro -> content = malloc(MAX_LINE_LEN);
    CHECK_MEM_ALLOC(macro -> content)
    macro -> content[0] = '\0';
}

/* adds a macro to the macro table. since its given that macro definitions dont have any errors - no error checks for macros */
void addMacro(Macro newMacro)
{
    macroCount++;
    if (macroTable == NULL) {
        macroTable = (Macro*)malloc(sizeof(Macro));
        CHECK_MEM_ALLOC(macroTable)
    }
    else {
        macroTable = (Macro*)realloc(macroTable, macroCount * sizeof(Macro));
        CHECK_MEM_ALLOC(macroTable)
    }
    macroTable[macroCount - 1] = newMacro;
}

/* initializes a macro (with initMacro()), reads its contents and enters it to the macro table
 * when called, fptr will point to the first line in the macro body */
void readMacro(FILE *fptr, char *name, int *lineNum)
{
    char line[MAX_LINE_LEN], lineCopy[MAX_LINE_LEN];
    int length = 0; /* lineCopy is the same as line but with trimmed blanks - the line WITH blanks will be copied to currMacro's content */
    Macro currMacro;
    initMacro(&currMacro);
    strcpy(currMacro.name , name);
    fgets(line, MAX_LINE_LEN, fptr);
    strcpy(lineCopy, line);
    trimBlanks(lineCopy);

    while (strcmp(lineCopy, "endmcro\n") != 0)
    {
        lineNum++;  /* updates the index of the line currently read from source file */
        if ((length += strlen(line)) > MAX_LINE_LEN) {
            currMacro.content = (char*)realloc(currMacro.content, length  );
            CHECK_MEM_ALLOC(currMacro.content)
        }
        strcat(currMacro.content, line);
        fgets(line, MAX_LINE_LEN, fptr);
        strcpy(lineCopy, line);
        trimBlanks(lineCopy);
    }
    addMacro(currMacro);
}

/* checks whether the current line being read is a macro placement, if so returns its index in the table and -1 if otherwise */
int checkMacroPlacement(char* line)
{
    int i = 0;
    trimLastChar(line); /* removes the '\n' from the line to be compared with the macro table */
    while (i < macroCount)
    {
        if (strcmp(macroTable[i].name, line) == 0)
            return i;
        i++;
    }
        return -1;
}

/* this function is called in the end of post-processing, frees the dynamically allocated memory regarding macros */
void freeMacroTable()
{
    int i;
    if (macroTable != NULL && macroCount != 0)
    {
        for (i = 0; i < macroCount; i++)
            free(macroTable[i].content);
        free(macroTable);            
    }
}
