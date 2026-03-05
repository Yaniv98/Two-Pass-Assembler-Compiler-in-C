#ifndef MEMORY_H
#define MEMORY_H

#include "master_header.h"

void initICDC();
void addToDataArr(Line line);
void addToInstructionsArr(Line line);
void addToEntryArr(Line line);
void addToExternArr(Line line);
void freeAll();

#endif
