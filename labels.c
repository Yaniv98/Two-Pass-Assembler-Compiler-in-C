#include <ctype.h>

#include "master_header.h"

extern int IC, DC, labelCount, entryCount, externCount;
extern Label *labelTable;
extern char instructionsArray[MEMORY_SIZE][MAX_LABEL_LEN + 1];
extern char **entriesArray, **externsArray;
extern Operation operations[NUM_OF_OPS];

/* validates a given label name, if its valid returns TRUE and FALSE if otherwise */
boolean validLabelName(Line line, int opdInd)
{
    int i, len = strlen(line.words[opdInd]); /* 0 if entExtDir = FALSE and 1 if otherwise */
    char *labelName = malloc(len + 1);
    CHECK_MEM_ALLOC(labelName)

    strcpy(labelName, line.words[opdInd]);
    if (opdInd == 0) /* this means its a new label definition, in this case labelName[lastChar] = ':' */
        trimLastChar(labelName);

    if (!isalpha(labelName[0])) { /* first char of a label's name must be alphabetical */
        raiseInvalidLabelName(labelName, line.index, "(first char is not alphabetical)");
        free(labelName);
        return FALSE;
    }

    if (len > MAX_LABEL_LEN) {
        raiseInvalidLabelName(labelName, line.index, "(exceeding max label length)");
        free(labelName);
        return FALSE;
    }
    else
    {
        for (i = 0; i < NUM_OF_OPS; i++) /* checks if the label name isnt the same as an operation */
            if (strcmp(labelName, operations[i].name) == 0) {
                if (opdInd == 0)
                    raiseInvalidLabelName(labelName, line.index, "(same as an operation)");
                free(labelName);
                return FALSE;
            }
        for (i = 1; i < len - 1; i++)   /* checks if all the chars in the label name are alphabetical or digits */
            if (!isalnum(labelName[i])) {
                raiseInvalidLabelName(labelName, line.index, "(invalid char in label definition)");
                free(labelName);
                return FALSE;
            }
        if (opdInd == 0) /* otherwise: the function was called to check a label name in an .entry/.extern declaration */
            for (i = 0; i < labelCount; i++)    /* checks if the label currently defined was already defined elsewhere */
                if (strcmp(labelTable[i].name, labelName) == 0) {
                    raiseInvalidLabelName(labelName, line.index, "(a label with this name already exists)");
                    free(labelName);
                    return FALSE;
                }
    }
    free(labelName); 
    return TRUE;
}

/* adds a given label to the label table and sets its address and tag as required */
void handleLabelDeclaration(Line line)
{
    Label newLabel;
    strcpy(newLabel.name, line.words[0]);

    if (line.tag == DATA || line.tag == STRING) {
        newLabel.address = DC;
        newLabel.tag = DATA;
    }
    else if (line.tag == INSTRUCTION) {
        newLabel.address = IC;
        newLabel.tag = INSTRUCTION;
    }
    else /* invalid tag case is already handled in firstRun() so this case can only be ENTRY/EXTERN */
        return;

    labelCount++;
    
    labelTable = (Label*)realloc(labelTable, labelCount * sizeof(Label));

    CHECK_MEM_ALLOC(labelTable)
    labelTable[labelCount - 1] = newLabel;
}

/* this function is called in second pass and checks that all operands of a line that are direct (labels) are properly defined
 * in the file. returns TRUE if an error is found and FALSE if otherwise */
boolean validLabelUsage(Line line)
{
    int i, k, opdAmount = operations[line.opCode].operandAmount, opdInd = 1 + (int)line.labelDeclared;
    boolean returnVal = FALSE; /* instead of returning on error discovery - will continue to look for errors in the line */
    if (opdAmount == 1 && line.dstType == DIRECT)
    {
            for (i = 0; i < labelCount && strcmp(labelTable[i].name, line.words[opdInd]) != 0; i++) {}
            for (k = 0; k < externCount && strcmp(externsArray[k], line.words[opdInd]) != 0; k++) {}
            if (i == labelCount && k == externCount) { /* a label used in the current line isnt a label/entry/extern (unknown) - error */
                raiseUnknownLabel(line, opdInd);
                returnVal = TRUE;
            }
    }
    if (opdAmount == 2)
    {
        if (line.srcType == DIRECT)
        {
            for (i = 0; i < labelCount && strcmp(labelTable[i].name, line.words[opdInd]) != 0; i++) {}
            for (k = 0; k < externCount && strcmp(externsArray[k], line.words[opdInd]) != 0; k++) {}
            if (i == labelCount && k == externCount) {
                raiseUnknownLabel(line, opdInd);
                returnVal = TRUE;
            }
        }
        if (line.dstType == DIRECT)
        {
            for (i = 0; i < labelCount && strcmp(labelTable[i].name, line.words[opdInd + 1]) != 0; i++) {}
            for (k = 0; k < externCount && strcmp(externsArray[k], line.words[opdInd + 1]) != 0; k++) {}
            if (i == labelCount && k == externCount) {
                raiseUnknownLabel(line, opdInd + 1);
                returnVal = TRUE;
            }
        }
    }
    return returnVal;
}

/* goes through the instructions array and encodes the required binary code in the cells that contain a label's name */
void encodeLabels()
{
    int i, k, preCodeNum;
    char *binaryVal;

    for (i = MEM_START_POINT; i < IC; i++)
    {
        for (k = 0; k < labelCount; k++) /* checks for .entry labels or labels that were defined in the file without an .entry declaration */
            if (strcmp(instructionsArray[i], labelTable[k].name) == 0)
            {
                preCodeNum = labelTable[k].address;
                preCodeNum <<= ARE_SIZE; /* moves the bits to the left 2 times to make room for A,R,E bits */
                preCodeNum |= RELOC_CODE; /* turns on bits 0,1 as required */
                binaryVal = convertIntToBinary(preCodeNum);
                strcpy(instructionsArray[i], binaryVal);
                free(binaryVal);
                break;
            }

        for (k = 0; k < externCount; k++) /* checks for .extern labels */
            if (strcmp(instructionsArray[i], externsArray[k]) == 0) {
                strcpy(instructionsArray[i], EXTERN_BINCODE); /* places ...001 as required */
                break;
            }
    }
}