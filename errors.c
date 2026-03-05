#include <stdio.h>

#include "master_header.h"

extern Operation operations[NUM_OF_OPS];
/* in case the terminal prompt to run the program contains too few arguments */
void raiseInvalidInput()
{
    fprintf(stdout, "Invalid input - no files specified. Terminating.\n");
    exit(0);
}

/* prints the file's name and the line number in which there was an error. this function is called by all other error reporting functions excluding raiseInvalidInput() */
void printErrLocation(int lineNum)
{
    fprintf(stdout, "ERROR: In file \"%s.%s\" line %d: ", getFileName(), POST_PROCESS_FILETYPE, lineNum);
}

/* this function is called when the current line read is longer than the max length */
void raiseExceedingMaxLineLen(Line line)
{
    printErrLocation(line.index);
    fprintf(stdout, "line exceeds maximum line length (%d)\n", MAX_LINE_LEN - 1);
}

/* this function is called when an error was found in a line's tag definition */
void raiseInvalidTag(Line line)
{
    int len = strlen(line.words[(int)line.labelDeclared]);
    printErrLocation(line.index);

    if (line.words[(int)line.labelDeclared][len - 1] == ',') {
        fprintf(stdout, "invalid comma placement after operation name\n");
        trimLastChar(line.words[(int)line.labelDeclared]);
    }
    else
        fprintf(stdout, "invalid order \"%s\"\n", line.words[line.labelDeclared]);
}

/* this function is called when an error was found in a .string directive line.
 * boolWordAmount acts as a boolean, is 1 if the function was called due to invalid argument amount and 0 if otherwise */
void raiseInvalidStringDirective(int lineNum, boolean boolWordAmount)
{
    printErrLocation(lineNum);
    if (boolWordAmount)
        fprintf(stdout, "too few arguments for .string directive\n");
    else
        fprintf(stdout, "missing '%c' in string directive\n", '"');
}

/* this function is called when defining a label which name has an error */
void raiseInvalidLabelName(char *labelName, int lineNum, char *reason)
{
    printErrLocation(lineNum);
    fprintf(stdout, "invalid label name \"%s\" %s\n", labelName, reason);
}

/* this function is called if an extra comma was found in a line */
void raiseExtraComma(Line line)
{
    printErrLocation(line.index);
    fprintf(stdout, "line contains extra commas\n");
}

/* this function is called when theres a missing comma anywhere in a line being read */
void raiseMissingComma(int lineNum, int wordNum, char* lineType)
{
    printErrLocation(lineNum);
    fprintf(stdout, "missing comma after word %d in %s\n", wordNum, lineType);
}

/* this function is called when a number in an instruction line or a .data line is out of legal range [-2048, 2047] */
void raiseNumRangeErr(Line line, int opdInd)
{
    printErrLocation(line.index);
    if (line.tag == INSTRUCTION)
        fprintf(stdout, "operand %d is out of legal range [%d,%d]\n", opdInd, MIN_INST_NUM_VAL, MAX_INST_NUM_VAL);
    else
        fprintf(stdout, "operand %d is out of legal range [%d,%d]\n", opdInd, MIN_DATA_NUM_VAL, MAX_DATA_NUM_VAL);
}

/* this function is called when theres a label before an .entry/.extern declaration, for example L1: .entry L2, raises a warning */
void raiseLabelTagWarning(Line line)
{
    char *tagName = line.tag == ENTRY ? ".entry" : ".extern"; /* used to print whats the line tag that caused this warning */
    fprintf(stdout, "WARNING: In file \"%s.%s\" line %d: ", getFileName(), POST_PROCESS_FILETYPE, line.index);
    fprintf(stdout, "meaningless label declaration \"%s\" in '%s' line\n", trimLastChar(line.words[0]), tagName); /* the label name has ':' after it */
}

/* this function is called when a line contains an empty label definition */
void raiseEmptyLabelDef(Line line)
{
    printErrLocation(line.index);
    fprintf(stdout, "label \"%s\" declared but not defined\n", trimLastChar(line.words[0])); /* removes the ':' */
}

/* this function is called when extraneous text was found in a line */
void raiseExtraText(int lineNum)
{
    printErrLocation(lineNum);
    fprintf(stdout, "extraneous text or too many arguments\n");
}

/* this function is called when an instruction line has too few arguments */
void raiseFewerArgs(Line line)
{
    printErrLocation(line.index);
    fprintf(stdout, "too few arguments for instruction %s (expected %d)\n", operations[line.opCode].name, operations[line.opCode].operandAmount);
}

/* this function is called when a line's operand is invalid for the reason specified in char* reason */
void raiseInvalidOpd(char* operand, int lineNum, char* reason)
{
    char *opdCopy = malloc((strlen(operand) + 1) * sizeof(char));
    CHECK_MEM_ALLOC(opdCopy)
    printErrLocation(lineNum);
    strcpy(opdCopy, operand);
    if (opdCopy[strlen(opdCopy) - 1] == ',')
        trimLastChar(opdCopy);
    if (strcmp(opdCopy, "@") == 0)
        fprintf(stdout, "invalid use of registry\n");
    else
        fprintf(stdout, "invalid operand \"%s\": %s\n", opdCopy, reason);
    free(opdCopy);
}

/* this function is called when a .entry/.extern line has an error.
 * if type = NONE the function was called due to insufficient arguments */
void raiseInvalidEntExtDirective(Line line, Assignment_type type, char* reason)
{
    char* tag = line.tag == ENTRY ? ".entry" : ".extern";
    printErrLocation(line.index);
    if (type == NONE)
        fprintf(stdout, "%s in %s directive\n", reason, tag);
    else
        fprintf(stdout, "invalid \"%s\" directive: %s", tag, reason);
}

/* this function is called if it was found that a label declared as .extern has a redefinition in the current file being worked on */
void raiseExtLabelRedef(Line line)
{
    printErrLocation(line.index);
    fprintf(stdout, "redefinition of label \"%s\", was declared as 'extern'\n", line.words[0]);
}

/* this function is called if a label declared as .entry isnt defined (as it must be) in the file */
void raiseMissingLabelDef(char *entry)
{
    int i;
    int size = findNextSeperation(entry, 0);
    char *name = malloc(size+1);
    CHECK_MEM_ALLOC(name)
    
    for(i=0;i<size;i++)
        name[i] = entry[i];
    name[size] = '\0';    
    
    printErrLocation(atoi(entry + size));
    fprintf(stdout, "file is missing label definition for '.entry' declaration \"%s\"\n", name);
    free(name);
}

/* this function is called when an instruction line has an unidentified label as an operand
 * unidentified: neither declared as .extern nor defined in current file */
void raiseUnknownLabel(Line line, int opdInd)
{
    printErrLocation(line.index);
    fprintf(stdout, "unknown operand or undeclared label \"%s\"\n", line.words[opdInd]);
}