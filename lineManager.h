#ifndef LINEMANAGER_H
#define LINEMANAGER_H
#include "master_header.h"

typedef struct Line
{
    char content[MAX_LINE_LEN], **words; /* content is the line's content as a single string, words is an array of strings that is every word separately */
    int wordCount, index, opCode;   /* wordCount is the amount of separate words in the line, opCode is the code of the line's operation (if tag = instruction) */
    Directive_tag tag;  /* .data, .string, .entry, .extern, instruction, invalid tag */
    Assignment_type srcType, dstType;   /* immediate, direct, direct register */
    boolean labelDeclared;  /* equals 1 if a label was declared in the line and 0 if otherwise */
} Line;

Line* initLine();
void resetLine(Line *line);
void freeLine(Line *line);
boolean validStringDirective(Line line);
boolean validDataDirective(Line line);
boolean validLine(Line line);
boolean setLineProps(Line *line, boolean *extraTextFlag, boolean *commentFlag);

#endif