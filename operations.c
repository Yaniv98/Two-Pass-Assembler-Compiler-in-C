#include "master_header.h"

/* list of the operations and their properties */
Operation operations[NUM_OF_OPS] = {
/* {name, opCode, operandAmount, sourceTypes, destTypes} */
{"mov", 0, 2, {IMMEDIATE, DIRECT, DIRECT_REG}, {DIRECT, DIRECT_REG}},
{"cmp", 1, 2, {IMMEDIATE, DIRECT, DIRECT_REG}, {IMMEDIATE, DIRECT, DIRECT_REG}},
{"add", 2, 2, {IMMEDIATE, DIRECT, DIRECT_REG}, {DIRECT, DIRECT_REG}},
{"sub", 3, 2, {IMMEDIATE, DIRECT, DIRECT_REG}, {DIRECT, DIRECT_REG}},
{"not", 4, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"clr", 5, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"lea", 6, 2, {DIRECT}, {DIRECT, DIRECT_REG}},
{"inc", 7, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"dec", 8, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"jmp", 9, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"bne", 10, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"red", 11, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"prn", 12, 1, {NONE}, {IMMEDIATE, DIRECT, DIRECT_REG}},
{"jsr", 13, 1, {NONE}, {DIRECT, DIRECT_REG}},
{"rts", 14, 0, {NONE}, {NONE}},
{"stop", 15, 0, {NONE}, {NONE}}
};

/* checks if a given string is the same as an operation name, if it is returns its number (0 - 15) and -1 if otherwise */
int getOpCode(Line line)
{
    int i;
    for (i = 0; i < NUM_OF_OPS; i++)
    {
        if (line.labelDeclared) {
            if (line.wordCount > 1 && strcmp(line.words[1], operations[i].name) == 0) /* this ensures the line isnt an empty label definition */
                return i;
        }
        else if (strcmp(line.words[0], operations[i].name) == 0)
            return i;
    }
    return -1;
}

/* this function validates a line of type INSTRUCTION, is called by validLine() in lineManager.c */
boolean validOp(Line line)
{
    int i, opdAmount = operations[line.opCode].operandAmount, opdInd = 1 + (int)line.labelDeclared;
    boolean invalidSrc, invalidDst;

    if (opdAmount + line.labelDeclared + 1 > line.wordCount) { /* no need to check >wordCount because of checkExtraText() in firstRun() */
        raiseFewerArgs(line);
        return FALSE;
    }

    if (opdAmount > 0)   /* no need to validate operands in 0 operands instructions */
    {
        invalidSrc = TRUE;
        invalidDst = TRUE;
        if (opdAmount == 1) { /* this control flow section is to prevent reporting both "invalid assignment type" and "invalid reg name" in case DIRECT_REG is valid */
            if (line.words[opdInd][0] == '@')
                line.dstType = DIRECT_REG;
        }
        else /* opdAmount = 2 */
        {
            if (line.words[opdInd][0] == '@')
                line.srcType = DIRECT_REG;
            if (line.words[opdInd + 1][0] == '@')
                line.dstType = DIRECT_REG;
        }

        for (i = 0; i < NUM_OF_TYPES; i++) /* each operation has an array in size 4 of valid types for each operand */
        {
            if (operations[line.opCode].sourceTypes[i] == line.srcType)
                invalidSrc = FALSE;
            if (operations[line.opCode].destTypes[i] == line.dstType)
                invalidDst = FALSE;
        }
        if (opdAmount == 2)
        {
            if (line.srcType == INVALID_TYPE)
                for (i = 0; i < NUM_OF_OPS; i++)
                    if (strcmp(line.words[opdInd], operations[i].name) == 0)
                        return FALSE;
            if (line.dstType == INVALID_TYPE)
                for (i = 0; i < NUM_OF_OPS; i++)
                    if (strcmp(line.words[opdInd + 1], operations[i].name) == 0)
                        return FALSE;

            if (line.words[opdInd][0] == '@')
                line.srcType = DIRECT_REG;
            if (line.words[opdInd + 1][0] == '@')
                line.dstType = DIRECT_REG;

            if (invalidSrc)
                raiseInvalidOpd(line.words[opdInd], line.index, "invalid assignment type");
            if (invalidDst)
                raiseInvalidOpd(line.words[opdInd + 1], line.index, "invalid assignment type");

            if (line.words[opdInd][strlen(line.words[opdInd]) - 1] != ',')
                raiseMissingComma(line.index, opdInd, "2 operands instruction");

            if (!invalidSrc && line.srcType == IMMEDIATE && (atoi(line.words[opdInd]) < MIN_INST_NUM_VAL || atoi(line.words[opdInd]) > MAX_INST_NUM_VAL)) {
                raiseNumRangeErr(line, opdInd);
                invalidSrc = TRUE;
            }

            if (!invalidDst && line.dstType == IMMEDIATE && (atoi(line.words[opdInd + 1]) < MIN_INST_NUM_VAL || atoi(line.words[opdInd + 1]) > MAX_INST_NUM_VAL)) {
                raiseNumRangeErr(line, opdInd + 1);
                invalidDst = TRUE;
            }
        }
        else /* opAmount = 1 */
        {
            if (line.dstType == INVALID_TYPE)
                for (i = 0; i < NUM_OF_OPS; i++)
                    if (strcmp(line.words[opdInd], operations[i].name) == 0)
                        return FALSE;

            if (line.words[opdInd][0] == '@')
                line.dstType = DIRECT_REG;

            if (line.dstType == DIRECT_REG && !checkRegName(line.words[opdInd]))
                raiseInvalidOpd(line.words[opdInd], line.index, "not a registry name");

            if (invalidDst)
                raiseInvalidOpd(line.words[opdInd], line.index, "invalid assignment type");
            if (!invalidDst && line.dstType == IMMEDIATE && (atoi(line.words[opdInd]) < MIN_INST_NUM_VAL || atoi(line.words[opdInd]) > MAX_INST_NUM_VAL)) {
                raiseNumRangeErr(line, opdInd);
                invalidDst = TRUE;
            }
        }
        if (invalidSrc || invalidDst) /* this is done separately to report all errors */
            return FALSE;
    }
    return TRUE;
}

/* this functions encodes the first binary word of an INSTRUCTION line */
char* encodeOp(Line line)
{
    int opdBinCode, wordBinCode = 0, numOfShifts = WORD_SIZE;
/* explanation - every operand encoding into the operation's binary code has 3 steps (initially all the bits of the op's bin code are set to 0):
* 1. opdBinCode gets the binary code of the current operand's type
* 2. using bitwise left shifts, the operand's bin code is moved to the right spot (11-9 for src, 8-5 for opCode, 4-2 for dest, ARE are always 0)
* 3. finally, the required bits are turned on in the op's binary code variable using bitwise OR
 * this process is done separately for each of the operands, no matter the operation (zeros will remain where necessary).
 * numOfShifts represents the number of shifts needed to align the bits into the right place. the logic:
 * its initialized to the size of the whole binary code, and then before every step is made the length of every operand in bits is
 * subtracted from numOfShifts - and thats the required number of left shifts. */
    numOfShifts -= OPERAND_SIZE;
    opdBinCode = line.srcType;
    opdBinCode <<= numOfShifts;
    wordBinCode |= opdBinCode;

    numOfShifts -= OPERATION_SIZE;
    opdBinCode = line.opCode;
    opdBinCode <<= numOfShifts;
    wordBinCode |= opdBinCode;

    numOfShifts -= OPERAND_SIZE;
    opdBinCode = line.dstType;
    opdBinCode <<= numOfShifts;
    wordBinCode |= opdBinCode;

    return convertIntToBinary(wordBinCode);
}
