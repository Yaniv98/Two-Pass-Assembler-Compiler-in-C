#ifndef ERRORS_H
#define ERRORS_H

#include "master_header.h"

void raiseInvalidInput();
void raiseExceedingMaxLineLen(Line line);
void raiseInvalidTag(Line line);
void raiseInvalidStringDirective(int lineNum, boolean boolWordAmount);
void raiseInvalidLabelName(char *labelName, int lineNum, char *reason);
void raiseExtraComma(Line line);
void raiseLabelTagWarning(Line line);
void raiseEmptyLabelDef(Line line);
void raiseExtraText(int lineNum);
void raiseMissingComma(int lineNum, int wordNum, char* lineType);
void raiseNumRangeErr(Line line, int opdInd);
void raiseFewerArgs(Line line);
void raiseInvalidOpd(char* operand, int lineNum, char* reason);
void raiseInvalidEntExtDirective(Line line, Assignment_type type, char* reason);
void raiseExtLabelRedef(Line line);
void raiseMissingLabelDef(char *entry);
void raiseUnknownLabel(Line line, int opdInd);

#endif
