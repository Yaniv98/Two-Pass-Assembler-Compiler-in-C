#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include "master_header.h"

int findNextSeperation(char *line, int i);
int findNextWord(char *line, int i);
void trimBlanks(char *line);
char** splitLine(Line *line);
boolean checkExtraText(Line line);
void trimPunctuations(Line *line);
int convertStrToInt(char* operand);
char* trimLastChar(char* str);

#endif