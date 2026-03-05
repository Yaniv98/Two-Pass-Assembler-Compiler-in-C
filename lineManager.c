#include <ctype.h>

#include "master_header.h"

extern char **entriesArray, **externsArray;
extern int entryCount, externCount;

/* initializes all the Line struct members */
Line* initLine()
{
    Line *line = (Line*)malloc(sizeof(Line));
    CHECK_MEM_ALLOC(line)
    line -> content[0] = '\0';
    line -> words = (char**)malloc(sizeof(char*));
    CHECK_MEM_ALLOC(line->words)
    line -> wordCount = 0;
    line -> index = 1;
    line -> srcType = NONE; /* a convenient default */
    line -> dstType = NONE; /* a convenient default */
    line -> labelDeclared = FALSE; /* a convenient default */
    line -> opCode = -1;    /* if the line is an instruction this will be updated accordingly */

    return line;
}

/* frees the dynamically allocated memory in Line struct */
void freeLine(Line *line)
{
    int i;
    for (i = 0; i < line -> wordCount; i++)
        free(line->words[i]);
    free(line->words);
    free(line);
}

/* sets the line's directive tag */
void setLineTag(Line *line)
{   /* if a label was declared in current line labelDeclared = 1 and 0 if otherwise, hence words[labelDeclared] = words[0/1] accordingly */
    if (strcmp(line -> words[(int)line -> labelDeclared], ".data") == 0)
        line -> tag = DATA;
    else if (strcmp(line -> words[(int)line -> labelDeclared], ".string") == 0)
        line -> tag = STRING;
    else if (strcmp(line -> words[(int)line -> labelDeclared], ".entry") == 0)
        line -> tag = ENTRY;
    else if (strcmp(line -> words[(int)line -> labelDeclared], ".extern") == 0)
        line -> tag = EXTERN;
    else if (getOpCode(*line) != -1)
        line -> tag = INSTRUCTION;
    else
        line -> tag = INVALID_TAG;
}

/* returns FALSE if the line received as a parameter is blank/a comment, TRUE if otherwise */
boolean setLineProps(Line *line, boolean *extraTextFlag, boolean *commentFlag)
{
    int i;
    trimBlanks(line -> content);
    if (strlen(line -> content) == 0 || line -> content[0] == ';') { /* line is empty or a comment */
        line -> index++;
        *commentFlag = TRUE;
        for(i = 0; i < line -> wordCount; i++)
            free(line -> words[i]);
        return FALSE;
    }

    line -> words = splitLine(line);
    line -> labelDeclared = (line -> words[0][strlen(line -> words[0]) - 1] == ':') ? TRUE : FALSE;
    if (line -> labelDeclared) {
        if (line -> wordCount > 1) /* handles case of empty label definition */
            setLineTag(line);
        else
            raiseEmptyLabelDef(*line);
    }
    else
        setLineTag(line);

    *extraTextFlag = checkExtraText(*line);

    if (line -> tag == INSTRUCTION) {
        line -> opCode = getOpCode(*line);
        assignOpdTypes(line);
    }
    *commentFlag = FALSE;
    return TRUE;
}

/* resets the Line members for the next iteration of the loop in first/second pass */
void resetLine(Line *line)
{
    int i;
    line -> content[0] = '\0';
    for (i = 0; i < line -> wordCount; i++)
       free(line -> words[i]);
    line -> wordCount = 0;
    line -> opCode = -1;
    line -> srcType = NONE;
    line -> dstType = NONE;
    line -> labelDeclared = FALSE;
}

/* this function is called when a line is a .data line, validates everything necessary for a .data directive. returns TRUE if the line is valid and FALSE if otherwise */
boolean validDataDirective(Line line)
{
    int i = line.labelDeclared ? 2 : 1, currLen;

    while (i < line.wordCount)
    {
        if (!validImmediateOpd(line.words[i], line.words[i][0] == '+' || line.words[i][0] == '-', line.index))
            return FALSE; /* if one of the operands isnt a valid number */

        currLen = strlen(line.words[i]);
        if (i < line.wordCount - 1 && line.words[i][currLen - 1] != ',') {
            raiseMissingComma(line.index, i + 1, "data directive");
            return FALSE;
        }
        if (atoi(line.words[i]) < MIN_DATA_NUM_VAL || atoi(line.words[i]) > MAX_DATA_NUM_VAL) {
            raiseNumRangeErr(line, i);
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

/* this function is called when a line is a .string line, validates everything necessary for a .string directive. returns TRUE if the line is valid and FALSE if otherwise */
boolean validStringDirective(Line line)
{
    int opdInd = 1 + (int)line.labelDeclared, lastChar = strlen(line.words[opdInd]) - 1;
    if (line.wordCount < 2) { /* its not >2 - was already checked in the call for checkExtraText() in firstRun() */
        raiseInvalidStringDirective(line.index, TRUE);
        return FALSE;
    }
    if (line.words[opdInd][0] != '"' || line.words[opdInd][lastChar] != '"') {/* str[len - 1] = '\n' and str[len - 2] should be '"' */
        raiseInvalidStringDirective(line.index, FALSE);
        return FALSE;
    }
    return TRUE;
}

/* this function is called when a line is a .entry/.extern line, validates everything necessary for a these directives. returns TRUE if the line is valid and FALSE if otherwise */
boolean validEntExtDirective(Line line)
{
    int i;
    char *reason, *temp; /* reason = the reason of a possible error, temp = utility string */
    if (line.wordCount < 2 + line.labelDeclared) {
        raiseInvalidEntExtDirective(line, NONE, "too few arguments");
        return FALSE;
    }
    else /* line.wordCount >= 2, more specifically = 3 since there isnt extra text if this function is called */
    {
            if (!validLabelName(line, 1 + (int)line.labelDeclared)) { /* checks if the operand read isnt a valid label name */
                raiseInvalidEntExtDirective(line, line.dstType, "invalid label name");
                return FALSE;
            }
            for (i = 0; i < entryCount; i++) /* checks for multiple declarations or if previously declared with another tag */
                if (strncmp(line.words[1], entriesArray[i], strlen(line.words[1])) == 0)
                {
                    if (line.tag == EXTERN) /* declared with another tag */
                        temp = "\" was already declared as entry";
                    else /* multiple declarations */
                        temp = "\" was already declared";

                    i = strlen("label \"") + strlen(line.words[1]) + strlen(temp) + 1; /* i no longer needed - used as utility variable */
                    reason = malloc(i * sizeof(char));
                    CHECK_MEM_ALLOC(reason)
                    sprintf(reason, "label \"%s%s", line.words[1], temp);
                    raiseInvalidEntExtDirective(line, line.dstType, reason);
                    free(reason);
                    return FALSE;
                }
            for (i = 0; i < externCount; i++) /* same as previous loop but for externsArray */
                if (strcmp(line.words[1], externsArray[i]) == 0)
                {
                    if (line.tag == ENTRY) /* declared with another tag */
                        temp = "\" was already declared as external";
                    else
                        temp = "\" was already declared";

                    i = strlen("label \"") + strlen(line.words[1]) + strlen(temp) + 1; /* i no longer needed - used as utility variable */
                    reason = malloc(i * sizeof(char));
                    CHECK_MEM_ALLOC(reason)
                    sprintf(reason, "label \"%s%s", line.words[1], temp);
                    raiseInvalidEntExtDirective(line, line.dstType, reason);
                    free(reason);
                    return FALSE;
                }
        }
    return TRUE;
}

/* this function validates any type of given line by using the other validation functions as required */
boolean validLine(Line line)  /* add already existing extern/entry declarations */
{
    int i;
    if (line.labelDeclared && (!validLabelName(line, 0) || line.wordCount < 2))
        return FALSE;

    for (i = 0; i < line.wordCount; i++)
    {
        if (strcmp(line.words[i], ",") == 0) { /* if an extra comma was found, line.words after splitLine() will contain a word "," */
            raiseExtraComma(line);
            return FALSE;
        }
        if (strcmp(line.words[i], "@") == 0) { /* if attempting to use the registries marker incorrectly, splitLine() return will contain a "@" word */
            return FALSE;
        }
    }
    switch (line.tag)
    {
        case ENTRY: /* .entry and .extern lines have the same validation requirements */
        case EXTERN:
        {
            if (line.labelDeclared)
            {
                raiseLabelTagWarning(line); /* raises a warning in case of "<label>: .entry/.extern <label>" */
                if (line.wordCount > 2 && validLabelName(line, 2))
                    return TRUE;
                else
                    return FALSE;
            }
            else
                return validEntExtDirective(line);
        }

        case DATA:
            return validDataDirective(line);

        case STRING:
            return validStringDirective(line);

        case INSTRUCTION:
            return validOp(line);

        case INVALID_TAG:
        {
            raiseInvalidTag(line);
            return FALSE;
        }
    }
    return TRUE;
}
