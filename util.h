#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include "master_header.h"

void checkFileOp(FILE *ptr, char* fileName);
char* convertIntToBinary(int num);
char *strrev(char *s);
void createObjFile(char *fileName);

static const char base64Table[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#endif