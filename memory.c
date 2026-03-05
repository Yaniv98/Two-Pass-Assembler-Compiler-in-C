#include <stdlib.h>

#include "master_header.h"

Label *labelTable = NULL;
int IC, DC;  /* IC = Instructions counter, DC = Data Counter. Each points to the next available spot in the instructions and data arrays. */
int entryCount = 0, externCount = 0, labelCount = 0;    /* counters for the amount of .entry/.extern/label declarations */
char instructionsArray[MEMORY_SIZE][MAX_LABEL_LEN + 1], dataArray[MEMORY_SIZE][WORD_SIZE + 1];
char **entriesArray = NULL, **externsArray = NULL;
extern Operation operations[NUM_OF_OPS];

/* Initializes the instructions and data counters to the memory start point */
void initICDC(){
    IC = MEM_START_POINT;
    DC = 0;
}

/* this functions adds the required codes to the data array, given the line is a .data/.string line */
void addToDataArr(Line line)
{
    int i, tempInt, opdInd = 1 + (int)line.labelDeclared; /* opdInd represents the index of the first operand in line's words array */
    char *binCode;

    if (line.tag == DATA) {
        for (i = opdInd; i < line.wordCount; i++) /* i = 1 if label declaration exists and 2 if otherwise */
        {
            tempInt = convertStrToInt(line.words[i]);
            binCode = convertIntToBinary(tempInt);
            strcpy(dataArray[DC], binCode);
            free(binCode);
            DC++;
        }
    }
    else {    /* tag == STRING */
        for (i = 1; line.words[opdInd][i] != '"'; i++)    /* i = 1 to skip the first '"' */
        {
            tempInt = (int)line.words[opdInd][i];
            binCode = convertIntToBinary(tempInt);
            strcpy(dataArray[DC], binCode);
            free(binCode);
            DC++;
        }
        binCode = convertIntToBinary(0);    /* this part null terminates the string in the data array */
        strcpy(dataArray[DC], binCode);
        free(binCode);
        DC++;
    }
}

/* this function adds the required codes or label names to the instructions array, given the line type is INSTRUCTION */
void addToInstructionsArr(Line line)
{
    int opdAmount = operations[line.opCode].operandAmount;
    char *opBinCode, **binCode;

    opBinCode = encodeOp(line); /* encodes the first binary word */
    strcpy(instructionsArray[IC], opBinCode);
    free(opBinCode);
    IC++;

    if (opdAmount > 0) {
        binCode = encodeOpds(line);
        if (strlen(binCode[1]) == 0) { /* this means theres just 1 added coded word in this line */
            strcpy(instructionsArray[IC], binCode[0]);
            IC++; 
        }
        else /* there are 2 added coded words in this line */
        {
            strncpy(instructionsArray[IC], binCode[0],WORD_SIZE);
            strncpy(instructionsArray[IC + 1], binCode[1],WORD_SIZE);
            IC += 2; 
            free(binCode[1]);
        }
        free(*binCode);
        free(binCode);
    }
    

}

/* this function adds a label name and the line number (which will be replaced later) to the entries array */
void addToEntryArr(Line line)
{
    entriesArray = (char**)realloc(entriesArray, (entryCount+1) * sizeof(char*));
    CHECK_MEM_ALLOC(entriesArray)
    entriesArray[entryCount] = malloc( strlen(line.words[1 + (int)line.labelDeclared]) + MAX_ADDRESS_LEN + 1);
    CHECK_MEM_ALLOC(entriesArray[entryCount])
    sprintf(entriesArray[entryCount], "%s\t%d", line.words[1 + (int)line.labelDeclared], line.index);
    entryCount++;
}

/* this function adds an extern label name to the externals array */
void addToExternArr(Line line)
{
    externsArray = (char**)realloc(externsArray, (externCount+1) * sizeof(char*));
    CHECK_MEM_ALLOC(externsArray)
    externsArray[externCount] = malloc( strlen(line.words[1 + (int)line.labelDeclared]) + 1 );
    CHECK_MEM_ALLOC(externsArray[externCount])
    strcpy(externsArray[externCount], line.words[1 + (int)line.labelDeclared]);
    externCount++;
}

/* this function is called in main(), frees the required memory when done using it entirely */
void freeAll()
{   /* note: instructions & data arrays don't have to be freed */
    int i;

        if (entriesArray != NULL) {
            for (i = 0; i < entryCount; i++)
                free(entriesArray[i]);
            free(entriesArray);
        }
        
        if (externsArray != NULL) {
            for (i = 0; i < externCount; i++)
                free(externsArray[i]);
            free(externsArray);
        }

    if (labelTable != NULL) /* frees labelTable  (if needed) */
        free(labelTable);
}
