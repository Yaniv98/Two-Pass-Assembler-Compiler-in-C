#include <ctype.h>

#include "master_header.h"

extern Operation operations[NUM_OF_OPS];

/* returns the index of the next blank or comma after line[i] */
int findNextSeperation(char *line, int i)
{
    if (line[i] == '"')
    {
        i++;
        while (line[i] != '"' && line[i] != '\n')
            i++;
    }
    else
        while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != ',')
            i++;
    return i;
}

/* returns the index of the next word in the line */
int findNextWord(char *line, int i)
{
    while (line[i] == ' ' || line[i] == '\t' || line[i] == '\n' /*|| line[i] == ','*/)
        i++;
    return i;
}

/* removes the last character from char* str */
char* trimLastChar(char* str)
{
    str[strlen(str) - 1] = '\0';
    return str;
}

/* removes punctuations (if they exist) from all the words in line -> words */
void trimPunctuations(Line *line)
{
    int i, lastCharInd;
    for (i = 0; i < line -> wordCount - 1; i++)
    {
        lastCharInd = strlen(line -> words[i]) - 1;
        if (line -> words[i][lastCharInd] == ':' || line -> words[i][lastCharInd] == ',')
            line -> words[i] = trimLastChar(line -> words[i]);
    }
}

/* removes head and tail blank chars of a line and places a newline char and a null terminator at the end */
void trimBlanks(char *line)
{
    int i = 0, j = findNextWord(line, 0);

    while (line[j] != '\0') /* removes the leading blanks */
    {
        line[i] = line[j];
        i++;
        j++;
    }

    line[i] = '\0';
    do /* moves j back to last non-blank char of the line */
        j--;
    while (j >= 0 && (line[j] == ' ' || line[j] == '\t' || line[j] == '\n'));

    line[j+1] = '\n';
    line[j+2] = '\0';
}

/* given a trimmed line: splits it into words (containing commas and colons etc.) and modifies the wordCount accordingly while running.
 * before returning the array of words, trims any leading and tailing blanks */
char** splitLine(Line *line)
{
    int i = 0, j;
    int lineLen = strlen(line -> content), currLen;
    char** splitResult = NULL;
    char* currWord = malloc(MAX_LINE_LEN+1);
    CHECK_MEM_ALLOC(currWord)
    line -> wordCount = 0;
    do {
        j = findNextSeperation(line -> content, i);
        if (line -> content[j] == '"')
            j++;
        strncpy(currWord, line -> content + i, j - i + 1);
        currWord[j - i] = '\0';
        currLen = strlen(currWord);
        line -> wordCount++;
        splitResult = (char**)realloc(splitResult, line->wordCount * sizeof(char*));
        CHECK_MEM_ALLOC(splitResult)
        i = findNextWord(line -> content, j);

        if (line -> content[i] == ',')
        {
            trimBlanks(currWord);
            if (currLen > 0 && currWord[currLen - 1] != ',')
                currWord[strlen(currWord) - 1] = ',';
            else
                *currWord = ',';    
            i = findNextWord(line -> content, i + 1);
        }
        splitResult[line -> wordCount - 1] = malloc(strlen(currWord) + 1);
        CHECK_MEM_ALLOC(splitResult[line -> wordCount - 1])
        strcpy(splitResult[line -> wordCount - 1], currWord);
    } while (i < lineLen && i != j);

    free(currWord);
    return splitResult;
}

/* checks if theres any extra text after the line ends, returns TRUE if there is and FALSE if otherwise */
boolean checkExtraText(Line line)
{
    int i, lastWordInd = line.wordCount - 1, lastWordLen = strlen(line.words[lastWordInd]);
    switch (line.tag)
    {
	case INVALID_TAG:
        case DATA:
        {
            if (!isdigit(line.words[lastWordInd][lastWordLen - 1])) {
                raiseExtraText(line.index);
                return TRUE;
            }
            break;
        }
        case STRING:
        {
            if (line.wordCount > (line.labelDeclared ? 3 : 2)) { /* expected: 3 words if a label exists and 2 if otherwise in a .string directive */
                raiseExtraText(line.index);
                return TRUE;
            }
            break;
        }
        case INSTRUCTION:
        {   /* getOpAmount(<opName in line>) + 1 for the op itself + 1 if a label exists (+0 if otherwise) */
            for (i = 0; i < line.wordCount; i++)
                if (strcmp(line.words[i], ",") == 0 || strcmp(line.words[i], "@") == 0)
                    break;

            if (i == line.wordCount && line.wordCount > operations[getOpCode(line)].operandAmount + 1 + (int)line.labelDeclared) {
                raiseExtraText(line.index);
                return TRUE;
            }
            break;
        }
        case ENTRY:
        case EXTERN:
        {
            if (!isalnum(line.words[lastWordInd][lastWordLen - 1])) { /* .entry/.extern dont have a limit on opd amount */
                raiseExtraText(line.index);
                return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

/* receives a string representing a number, copies it and returns it as an int */
int convertStrToInt(char* str)
{
    int result, resultLen = strlen(str);
    char* opdCopy = (char*)malloc(resultLen + 1);
    CHECK_MEM_ALLOC(opdCopy)
    strcpy(opdCopy, str);
    if (!isdigit(opdCopy[resultLen - 1]))
        opdCopy[resultLen - 1] = '\0';
    result = atoi(opdCopy);
    free(opdCopy);
    return result;
}
