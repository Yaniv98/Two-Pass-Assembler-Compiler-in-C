#include <ctype.h>

#include "master_header.h"

extern Operation operations[NUM_OF_OPS];

/* this function validates an immediate type operand, returns TRUE if its valid and FALSE if otherwise */
boolean validImmediateOpd(char* operand, boolean isSigned, int lineNum)
{
    int i;
    if (strlen(operand) == 1 && isSigned) {  /* operand is just '-' or '+' which is invalid */
        raiseInvalidOpd(operand, lineNum, "not a number/not an integer");
        return FALSE;
    }
    else
        for (i = 0; operand[i] != '\0'; i++) {
            if (i == 0 && isSigned)
                continue;
            if (!isdigit(operand[i]) && operand[i] != ',' && operand[i] != '\n') {
                raiseInvalidOpd(operand, lineNum, "not a number/not an integer");
                return FALSE;
            }
        }
    return TRUE;
}

/* this function validates a direct type operand, returns TRUE if its valid and FALSE if otherwise */
boolean validDirectOpd(char* operand, int lineNum)
{
    int i;
    if (isdigit(operand[0])) { /* direct operand cant start with a digit */
        raiseInvalidOpd(operand, lineNum, "invalid operand name");
        return FALSE;
    }
    for (i = 0; i < NUM_OF_OPS; i++)
        if (strcmp(operand, operations[i].name) == 0) { /* direct operand cant have the same name as an operation */
            raiseInvalidOpd(operand, lineNum, "same as an operation name");
            return FALSE;
        }
    return TRUE;
}

/* this function checks if a given string is a registry name, returns TRUE if it is and FALSE if otherwise */
boolean checkRegName(char* str)
{
    int i;
    const char registryNames[8][4] = {"@r0", "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7"};
    for (i = 0; i < NUM_OF_REGS; i++)
        if (strcmp(str, registryNames[i]) == 0)
            return TRUE;
    return FALSE;
}

/* gets an operand's assignment type (immediate/direct/direct register) and validates it with the validation functions */
Assignment_type getOpdType(char* operand, int lineNum)
{
    Assignment_type returnVal;
    boolean isSigned;
    char *opdCopy = malloc((strlen(operand) + 1) * sizeof(char));
    CHECK_MEM_ALLOC(opdCopy)
    strcpy(opdCopy, operand);
    if (opdCopy[strlen(opdCopy) - 1] == ',')
        opdCopy[strlen(opdCopy) - 1] = '\0';

    if ((isSigned = (opdCopy[0] == '-' || opdCopy[0] == '+') ? TRUE : FALSE) || isdigit(opdCopy[0]))  /* operand is possibly a number, immediate */
        returnVal = validImmediateOpd(opdCopy, isSigned, lineNum) ? IMMEDIATE : INVALID_TYPE;

    else if (opdCopy[0] == '@')
    {
        returnVal = checkRegName(opdCopy) ? DIRECT_REG : INVALID_TYPE;
        if (returnVal == INVALID_TYPE)
            raiseInvalidOpd(opdCopy, lineNum, "not a registry name");
    }
    else
        returnVal = validDirectOpd(opdCopy, lineNum) ? DIRECT : INVALID_TYPE;

    free(opdCopy);
    return returnVal;
}

/* assigns values to line's srcType and dstType */
void assignOpdTypes(Line *line)
{
    int opdInd = 1 + (int)line -> labelDeclared, opdAmount = operations[line -> opCode].operandAmount;

    if (opdAmount == 1)
        line -> dstType = getOpdType(line -> words[opdInd], line -> index);

    if (opdAmount == 2) {
        line -> srcType = getOpdType(line -> words[opdInd], line -> index);
        if (opdInd + 1 < line -> wordCount)
            line -> dstType = getOpdType(line -> words[opdInd + 1], line->index);
    }
}

/* this function encodes an operand in an INSTRUCTION line which has 1 operand */
char* encode1Opd(Line line)
{
    int binCode, opdInd = 1 + (int)line.labelDeclared;
    char *copy;
    if (line.dstType == DIRECT) {
        copy = malloc(strlen(line.words[opdInd]) + 1);
        CHECK_MEM_ALLOC(copy)
        strcpy(copy,line.words[opdInd]);
        return copy; /* if the operand is DIRECT it cant be encoded in first pass, meanwhile the "coding" will be the operand's name */
    }
       

    if (line.dstType == IMMEDIATE) {
        binCode = convertStrToInt(line.words[opdInd]);
        binCode <<= ARE_SIZE;
        return convertIntToBinary(binCode);
    }
    else /* dstType = DIRECT_REG */
    {
        binCode = convertStrToInt(line.words[opdInd] + 2); /* words[opdInd] + 2 points to the registry number */
        binCode <<= ARE_SIZE;
        return convertIntToBinary(binCode);
    }
}

/* this function encodes the operands in an INSTRUCTION line which has 2 operands */
char** encode2Opd(Line line)
{
    int opdInd = 1 + (int) line.labelDeclared, binCode1, binCode2;
    char *temp, **result = malloc(2 * sizeof(char*));
    CHECK_MEM_ALLOC(result)
    *result = malloc((WORD_SIZE + 1));
    CHECK_MEM_ALLOC(*result)
    result[1] = malloc((WORD_SIZE + 1));
    CHECK_MEM_ALLOC(result[1])

    switch (line.srcType)
    {
	case NONE: /* cases NONE & INVALID_TYPE are here to prevent compiler warnings, if it was the case this function wouldnt be called */
	case INVALID_TYPE:
        case IMMEDIATE:
        {
            binCode1 = convertStrToInt(line.words[opdInd]);
            binCode1 <<= ARE_SIZE;
            strcpy(result[0], temp = convertIntToBinary(binCode1));
            free(temp);
            break;
        }
        case DIRECT:    /* copies the label's name into the result array */
        {
            if (strlen(line.words[opdInd]) > WORD_SIZE + 1) {
                result[0] = realloc(result[0], (strlen(line.words[opdInd]) + 1)  );
                CHECK_MEM_ALLOC(result[0])
            }
            strcpy(result[0], line.words[opdInd]);
            break;
        }
        case DIRECT_REG:
        {
            binCode1 = convertStrToInt(line.words[opdInd] + 2); /* +2: sent parameter points to register's number */
            binCode1 <<= WORD_SIZE - REGISTRY_SIZE;
            strcpy(result[0], temp = convertIntToBinary(binCode1));
            free(temp);
            break;
        }
    }
    switch (line.dstType)
    {
	case NONE:
	case INVALID_TYPE:
        case IMMEDIATE:
        {
            binCode2 = convertStrToInt(line.words[opdInd + 1]);
            binCode2 <<= ARE_SIZE;
            strcpy(result[1], temp = convertIntToBinary(binCode2));
            free(temp);
            break;
        }
        case DIRECT:    /* copies the label's name into the result array */
        {
            if (strlen(line.words[opdInd + 1]) > WORD_SIZE + 1) {
                result[1] = realloc(result[1], (strlen(line.words[opdInd + 1]) + 1)  );
                CHECK_MEM_ALLOC(result[1])
            }
            strcpy(result[1], line.words[opdInd + 1]);
            break;
        }
        case DIRECT_REG: /* in this case theres only 1 added instruction word */
        {
            binCode2 = convertStrToInt(line.words[opdInd + 1] + 2); /* +2: sent parameter points to register's number */
            binCode2 <<= ARE_SIZE;
            if (line.srcType == DIRECT_REG)
            {
                binCode1 |= binCode2;
                strcpy(result[0], temp = convertIntToBinary(binCode1));
                free(result[1]);
                result[1] = "\0";   /* adds a marker that there isnt a second binary word */
            }
            else
                strcpy(result[1], temp = convertIntToBinary(binCode2));
            free(temp);
            break;
        }
    }
    return result;
}

/* binary operations logic is explained in encodeOp() in operations.c, opAmount != 0 when calling this function */
char** encodeOpds(Line line)
{
    char **result;

    if (operations[line.opCode].operandAmount == 1)
    {
        result = malloc(2 * sizeof(char*));
        CHECK_MEM_ALLOC(result)
        result[0] = encode1Opd(line);
        result[1] = "\0";    /* adds a marker that there isnt a second binary word */
    }
    else { /* opAmount = 2 */
        result = encode2Opd(line);
    }
    return result;
}
