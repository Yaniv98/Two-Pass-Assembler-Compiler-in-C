#include "master_header.h"

extern char **entriesArray, **externsArray;
extern char instructionsArray[MEMORY_SIZE][MAX_LABEL_LEN + 1];
extern Label *labelTable;
extern int labelCount, entryCount, externCount, IC;
static boolean outputEnt, outputExt;
extern Operation operations[NUM_OF_OPS];

/* this function is called by handleEntries(), creates the .ent file and writes into it */
void createEntriesFile(char *fileName)
{
    int i;
    FILE *fptr;
    sprintf(fileName, "%s.%s", getFileName(), ENT_FILETYPE);
    fptr = fopen(fileName, "w");
    checkFileOp(fptr, fileName);
    for (i = 0; i < entryCount; i++)
        fputs(entriesArray[i], fptr);

    fclose(fptr);
    checkFileOp(fptr, fileName);
}

/* handles the .entry declarations. if the definition for the label declared in the .entry line was found, the function changes
 * the number that represents the index of the declaration to the index of the line the label was defined at and calls createEntriesFile().
 * if otherwise: reports errors and changes the errorsExist flag in secondPass() */
void handleEntries(boolean *errorsExist, char *fileName)
{
    int i, k, labelLen;
    char address[MAX_ADDRESS_LEN + 2]; /* +2: '\t' + '\n' */
    char * currLabelname;

    for (i = 0; i <= entryCount; i++)
    {
        for (k = 0; k < labelCount && i < entryCount && !(*errorsExist); k++)
        {
            labelLen = strlen(labelTable[k].name);
            currLabelname =  labelTable[k].name;
            if (strncmp(entriesArray[i],currLabelname,labelLen) == 0)
            {
                outputEnt = TRUE;
                entriesArray[i][labelLen] = '\0';
                sprintf(address, "\t%d\n", labelTable[k].address);
                entriesArray[i] = realloc(entriesArray[i],strlen(entriesArray[i]) + strlen(address) + 1);
                CHECK_MEM_ALLOC(entriesArray[i])
                strcat(entriesArray[i], address);
                break;
            }
        }
        if (k == labelCount && labelCount > 0) /* indicates that a label declared as entry isnt defined in file - error */
        {
            raiseMissingLabelDef(entriesArray[i]);
            *errorsExist = TRUE;
            outputEnt = FALSE;
        }
    }
    if (outputEnt) /* will only be TRUE if no errors were found */
        createEntriesFile(fileName);
}

/* handles the .extern declarations. calling this function is done after doing the appropriate error checks for externals */
void handleExtern(Line line, FILE *extFptr)
{
    int i, opdAmount = operations[line.opCode].operandAmount, opdInd = 1 + (int)line.labelDeclared, n = strlen(line.words[opdInd]);
    char writeVal[MAX_LABEL_LEN + MAX_ADDRESS_LEN + 3]; /* +3: '\t' + '\n' + '\0' */

    for (i = 0; i < externCount; i++)
    {
        if (opdAmount == 1 && line.dstType == DIRECT && strncmp(line.words[opdInd], externsArray[i], n) == 0) {
            sprintf(writeVal, "%s\t%d\n", externsArray[i], line.index);
            fputs(writeVal, extFptr);
            outputExt = TRUE;
        }
        if (opdAmount == 2)
        {
            if (line.srcType == DIRECT && strncmp(line.words[opdInd], externsArray[i], n) == 0) {
                sprintf(writeVal, "%s\t%d\n", externsArray[i], line.index);
                fputs(writeVal, extFptr);
                outputExt = TRUE;
            }
            if (line.dstType == DIRECT && strncmp(line.words[opdInd + 1], externsArray[i], n) == 0) {
                sprintf(writeVal, "%s\t%d\n", externsArray[i], line.index);
                fputs(writeVal, extFptr);
                outputExt = TRUE;
            }
        }
    }
}

/* this function handles control flows and anything else required to do the second pass on the post-preprocessing .am file.
 * returns TRUE if any errors were discovered during the second pass and FALSE if otherwise */
boolean secondPass(FILE *readPtr)
{
    int i;
    boolean errorsExist = FALSE, prevIsComment = FALSE, invalidLine = FALSE , dummy; /* a dummy is required for calling the function setLineProps() */
    Line *currLine = initLine();
    FILE *fptr;
    char *fileName = malloc((strlen(getFileName()) + strlen(ENT_FILETYPE) + 2));
    CHECK_MEM_ALLOC(fileName)

    sprintf(fileName,"%s.%s",getFileName(),ENT_FILETYPE);
    handleEntries(&errorsExist, fileName);
    sprintf(fileName, "%s.%s", getFileName(), EXT_FILETYPE);
    fptr = fopen(fileName, "w");
    checkFileOp(fptr, fileName);

    while (fgets(currLine -> content, MAX_LINE_LEN, readPtr) != NULL)
    {

        if (!prevIsComment) {
            for (i = 0;i < currLine -> wordCount;i++)
                free(currLine -> words[i]);
            free(currLine -> words);
            currLine -> words = NULL;
        }

        if (!setLineProps(currLine, &dummy, &prevIsComment)) /* all lines are valid if this phase is reached */
            continue;
        trimPunctuations(currLine);

        if (currLine -> labelDeclared)
        {
            if (currLine -> tag == ENTRY || currLine -> tag == EXTERN) { /* no need to do anything with .entry/.extern lines in this phase */
                currLine -> index++;
                continue;
            }
            for (i = 0; i < externCount; i++)
                if (strcmp(currLine -> words[0], externsArray[i]) == 0) {
                    raiseExtLabelRedef(*currLine); /* this means that a label declared as .extern has a redefinition in the file - error */
                    invalidLine = TRUE;
                }
        }

        if (!invalidLine)
            invalidLine = validLabelUsage(*currLine);

        if (!invalidLine && currLine -> tag == INSTRUCTION && operations[currLine -> opCode].operandAmount > 0)
            handleExtern(*currLine, fptr);

        if (invalidLine)
            errorsExist = TRUE;

        currLine -> index++;
        invalidLine = FALSE; /* resets invalidLine for the next iteration */
        resetLine(currLine);
    }
    if (!errorsExist && (outputEnt || outputExt))
        encodeLabels();

    fclose(fptr);
    checkFileOp(fptr, fileName);
    if (!outputExt) /* by implementation - no need to do the same with .ent file */
        remove(fileName);
    free(fileName);

    for (i = 0; i < currLine -> wordCount; i++)
        free(currLine -> words[i]);

    if(currLine -> words!= NULL)
        free(currLine -> words);

    free(currLine);

    return errorsExist;
}