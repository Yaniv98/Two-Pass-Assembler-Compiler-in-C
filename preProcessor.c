#include "master_header.h"

extern Macro* macroTable;
/* opens .as file, reads and replaces the macros, writes the modified input into the .am file and returns it.
 * its given that macros dont contain any errors in their definitions and body so this function does not check for any errors */
FILE* preProcess(char* fileName)
{
    char *newFileName, lineCopy[MAX_LINE_LEN];
    Line *line = initLine();  /* line's index, tag or labelDeclared are irrelevant in this function */
    int i;
    FILE *writePtr=NULL, *readPtr=NULL;
    /* opening the source (.as) file */
    newFileName = (char*)malloc((strlen(fileName) + strlen(SRC_FILETYPE) + 2));
    CHECK_MEM_ALLOC(newFileName)
    sprintf(newFileName, "%s.%s", fileName, SRC_FILETYPE);
    readPtr = fopen(newFileName, "r");
    if (readPtr == NULL)
    {
        freeLine(line);
        freeMacroTable();
        free(newFileName);
        return NULL;
    }
    /* opening the post-preprocess (.am) file */
    sprintf(newFileName, "%s.%s", fileName, POST_PROCESS_FILETYPE);
    writePtr = fopen(newFileName, "w+"); /* w+ - writing to file in preprocessing, then reads from it later */
    checkFileOp(writePtr, newFileName);

    while ((fgets(line->content, MAX_LINE_LEN, readPtr)) != NULL)
    {
        strcpy(lineCopy, line->content);
        trimBlanks(lineCopy); /* removing the leading and tailing blanks */
        if (strlen(lineCopy) == 0) /* current line is a blank line */
        {
            fputs("\n", writePtr);
            line->index++;
            continue;
        }

        if (strncmp(lineCopy, "mcro", 4) == 0)  /* current line is a macro declaration, "mcro"'s length is 4 */
        {
            for(i=0;i<line->wordCount;i++)  /* to avoid memory leaks, the words array has to be freed before calling splitLine() */
                free(line->words[i]);
            free(line->words);
            trimBlanks(line -> content);
            line->words = splitLine(line);
            readMacro(readPtr, line->words[1], &(line->index)); /* after readMacro() readPtr points to the first line after endmcro */
            continue;
        }

        if ((i = checkMacroPlacement(lineCopy)) != -1)   /* current line is a macro placement */
        {
            fputs(macroTable[i].content, writePtr);
            line->index++;
            continue;
        }

        fputs(line->content, writePtr);
        line->index++;
    }
    /* freeing the required memory and resetting the file pointer */
    fclose(readPtr);    /* reading from source file is done */
    checkFileOp(readPtr, newFileName);
	freeLine(line);
    freeMacroTable();
    free(newFileName);
    rewind(writePtr);  /* resets file pointer to the beginning for next phase */
    return writePtr;
}
