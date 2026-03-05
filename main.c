#include "master_header.h"

static char *fileName;
extern char **entriesArray, **externsArray;
extern int entryCount,externCount,labelCount;
extern Label *labelTable;

/* main function. sends the files to preprocessing -> first pass -> second pass -> create .ob file. if errors were found in any of the phases, the program will not continue to the next phase */
int main(int argc, char *argv[])
{
    int i;
    boolean errorsExist;
    FILE *fptr; /* a pointer to the current file being worked on */
    if (argc < 2)   /* input in terminal must contain at least 1 argument other than ./<.exe name> */
        raiseInvalidInput();
    for (i = 1; i < argc; i++)
    {
        initMemoryArrays();
	    fileName = argv[i]; /* for later use in other functions that require the file's name */

        fptr = preProcess(fileName);
        if (fptr == NULL)
        {
            fprintf(stdout, "ERROR: file \"%s.%s\" wasn't found\n\n", fileName, SRC_FILETYPE);
            freeAll();
            continue;
        }
        errorsExist = firstPass(fptr);
        if (!errorsExist)
        {
            errorsExist = secondPass(fptr);
            fclose(fptr);
            freeAll();
            if (!errorsExist)
                createObjFile(fileName);
            else {
                fprintf(stdout, "Second pass complete. Terminating work on current file.\n\n");
            }
        }
        else
        {
            fclose(fptr);
            freeAll();
            fprintf(stdout, "First pass complete. Terminating work on current file.\n\n");
        }
    }
    return (int)!errorsExist;
}

void initMemoryArrays()
{
    entriesArray = NULL;
    entryCount = 0;
    externsArray = NULL;
    externCount = 0;

    labelTable = NULL;
    labelCount=0;
}

char* getFileName()
{
    return fileName;
}
