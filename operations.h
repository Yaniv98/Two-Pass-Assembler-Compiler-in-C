#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "master_header.h"

typedef struct Operation
{
    char name[MAX_OPERATION_LEN];  /* all operation names are in max length of 4, so 5 for the name + null termination */
    int opCode, operandAmount;
    Assignment_type sourceTypes[4];
    Assignment_type destTypes[4];
} Operation;

int getOpCode(Line line);
boolean validOp(Line line);
char* encodeOp(Line line);

#endif