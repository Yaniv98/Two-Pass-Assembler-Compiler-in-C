
#include "master_header.h"

void setLineTag(Line *line);
boolean validImmediateOpd(char *operand, boolean isSigned, int lineNum);
boolean validDirectOpd(char* operand, int lineNum);
Assignment_type getOpdType(char* operand, int lineNum);
void assignOpdTypes(Line *line);
char** encodeOpds(Line line);
boolean checkRegName(char* str);


