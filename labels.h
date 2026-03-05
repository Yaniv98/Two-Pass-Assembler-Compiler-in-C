#ifndef LABELS_H
#define LABELS_H

#include <stdlib.h>
#include "master_header.h"

/* name = label's name, address = index of containing memory cell, tag = DATA/INSTRUCTION */
typedef struct Label {
    char name[MAX_LABEL_LEN];
    int address;
    Directive_tag tag;
} Label;

boolean validLabelName(Line line, int opdInd);
void handleLabelDeclaration(Line line);
void encodeLabels();
boolean validLabelUsage(Line line);

#endif