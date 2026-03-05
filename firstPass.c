#include "master_header.h"

extern Label *labelTable;
extern int labelCount, IC;

/* this function handles control flows and anything else required to do the first pass on the post-preprocessing .am file.
 * returns TRUE if any errors were discovered during the first pass and FALSE if otherwise */
boolean firstPass(FILE* readFptr)
{
    int i;
    unsigned char c; /* this is only used if a line in the source file is longer than the max length */
    Line *currLine = initLine(); /* init stands for initialize */
    boolean errorsExist = FALSE, prevIsComment = FALSE, invalidLine = FALSE; /* default values that will be changed if necessary */
    initICDC();
    
    while (fgets(currLine -> content, MAX_LINE_LEN, readFptr) != NULL)
    {
        i = strlen(currLine -> content);
        if (i == MAX_LINE_LEN - 1 && currLine -> content[i] != '\n' && currLine -> content[i] != -1)
        {
            invalidLine = TRUE;
            currLine -> content[0] = '\0';
            do
                c = fgetc(readFptr);
            while (c != '\n' && c != EOF_MARK);
            if (c == '\n')
                c = fgetc(readFptr); /* this increments the file pointer by 1 and it now points to the next line in source file */

            errorsExist = TRUE;
            raiseExceedingMaxLineLen(*currLine);
            currLine -> index += 2;
            continue;
        }

        if (!prevIsComment){ /* if the previous line read is a comment/blank: splitLine wasnt called on line.words in setLineProps() so freeing it will cause memory corruption */
            for(i = 0; i < currLine -> wordCount; i++)
                free(currLine -> words[i]);
            free(currLine -> words);
            currLine->words = NULL;
        }   

        if (!setLineProps(currLine, &invalidLine, &prevIsComment)) /* evaluates to TRUE if line is blank or a comment */
            continue;

        if (invalidLine)
            errorsExist = TRUE;

        if (validLine(*currLine) && !invalidLine) /* if errorsExist = TRUE - errors were found in current/previous lines in the code */
        {   /* if validLine returns TRUE - line is valid in every aspect. even if !errorsExist, errors will still be reported by validLine */
            trimPunctuations(currLine);
            
            if (currLine -> labelDeclared)
                handleLabelDeclaration(*currLine);
            
            switch(currLine->tag) /* labelDeclared = TRUE & tag = ENTRY/EXTERN case already handled, so was tag = INVALID */
            {
		        case INVALID_TAG: /* this is here only to avoid compiler warning about case not being handled, if that was the case validLine would return FALSE */
                case ENTRY:
                {
                    addToEntryArr(*currLine);
                    break;
                }
                case EXTERN:
                {
                    addToExternArr(*currLine);
                    break;
                }
                case DATA: /* same handling for DATA & STRING cases */
                case STRING:
                {
                    addToDataArr(*currLine);
                    break;
                }
                case INSTRUCTION:
                    addToInstructionsArr(*currLine);
            }
        }
        else if (!(currLine -> labelDeclared && (currLine -> tag == ENTRY || currLine -> tag == EXTERN)))
            errorsExist = TRUE; /* prevents reporting a fatal error on "<label>: .entry/.extern" and killing program after first pass */

        currLine -> index++;
        invalidLine = FALSE; /* a reset for the next iteration */
        resetLine(currLine);
    }

    for (i = 0; i < labelCount; i++) /* adds IC to all DATA labels to code them after the instructions in .ob file as required */
        if (labelTable[i].tag == DATA)
            labelTable[i].address += IC;
    rewind(readFptr);   /* resets the pointer to the beginning of the file for next phase */
    for (i = 0; i < currLine -> wordCount; i++)
        free(currLine -> words[i]);
    if(currLine -> words!= NULL)
        free(currLine -> words);
    free(currLine);
    
    return errorsExist;
}
