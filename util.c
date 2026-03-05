#include <math.h>

#include "master_header.h"

extern int IC, DC;
extern char instructionsArray[MEMORY_SIZE][MAX_LABEL_LEN + 1], dataArray[MEMORY_SIZE][WORD_SIZE + 1];

/* receives a string representing a binary word and adds 1 to it */
char* add_one(char* bin_str) {
    int i, carry = 1;
    for (i = WORD_SIZE - 1; i >= 0; i--)
    {
        if (bin_str[i] == '0' && carry) {
            bin_str[i] = '1';
            carry = 0;
        }
        else if (bin_str[i] == '1' && carry) {
            bin_str[i] = '0';
            carry = 1;
        }
    }
    return bin_str;
}

/* this function takes an int as a parameter and returns a translation of it to binary */
char* convertIntToBinary(int num)
{
    int i, k;
    char* binWord = malloc(WORD_SIZE + 1); /* +1 to null-terminate the string */
    boolean negativeFlag = num < 0 ? TRUE : FALSE;
    CHECK_MEM_ALLOC(binWord)

    for (i = 0; i < WORD_SIZE; i++)
    {
        k = num % 2;
        if (k == 0)
            binWord[i] = '0';
        else
            binWord[i] = '1';
        num /= 2;
    }
    binWord[WORD_SIZE] = '\0';
    binWord = strrev(binWord);

    if (negativeFlag) { /* if the number received as a parameter is negative: */
        for (i = 0; i < WORD_SIZE; i++) /* functions similar to "~binWord" */
            binWord[i] = binWord[i] == '0' ? '1' : '0';
        binWord = add_one(binWord); /* finally adds 1 to the binary code */
    }
    return binWord;
}

/* checks whether a file was successfully opened/closed. if an unexpected error occured, shuts program down completely */
void checkFileOp(FILE *ptr, char* fileName)
{
    if (ptr == NULL)
    {
        printf("Unexpected error opening/closing file \"%s\".  Terminating program.\n", fileName);
        freeAll();
        freeMacroTable();
        exit(0);
    }
}

/* this function reverses the characters in a given string s and returns it when finished */
char *strrev(char *s)
{
    char *b,*e,t;
    if (s && *s) {
        b = s;
        e = s + strlen(s) - 1;
        while (b < e) {
            t = *b;
            *b++ = *e;
            *e-- = t;
        }
    }
    return s;
}

/* char *code64: a buffer for writing the base64 code
 * int i: the index of the binary word to be encoded to base64 from the instructions/data array
 * boolean dataCoding: is TRUE if currently encoding the data array and FALSE if encoding the instructions array
 * this function encodes the binary word from instructions/data array to base64 into the buffer code64 */
void convertBinToBase64(char *code64, int i, boolean dataCoding)
{
    int k, ch1 = 0, ch2 = 0;
    char *binCode;
    if (dataCoding)
        binCode = strrev(dataArray[i]); /* the binary word is reversed to be translated to base64 (for calculating the correct powers of 2) */
    else
        binCode = strrev(instructionsArray[i]);

    for (k = 0; k < WORD_SIZE/2; k++)
    {
        if (binCode[k] == '1')
            ch1 += (int)pow(2, k);
        if (binCode[k + WORD_SIZE/2] == '1')
            ch2 += (int)pow(2, k);
    }
    code64[0] = base64Table[ch2]; /* since the binary word was reversed, writing the coded 6 bits into code64 in reverse will give the right result */
    code64[1] = base64Table[ch1];
    code64[2] = '\n';
    code64[3] = '\0';
}

/* this function is called by main() if necessary (if no errors were found) */
void createObjFile(char *fileName)
{
    char *newFileName = malloc((strlen(fileName) + strlen(OBJ_FILETYPE) + 2));
    char headline[2 * MAX_ADDRESS_LEN + 3], code64[BASE64_LEN + 2]; /* +3: ' ' + '\n' + '\0', +2: '\0' + '\n' */
    int i;
    FILE *fptr;
    CHECK_MEM_ALLOC(newFileName)

    sprintf(newFileName, "%s.%s", fileName, OBJ_FILETYPE);
    sprintf(headline, "%d %d\n", IC - MEM_START_POINT, DC); /* the right amount of instructions is IC - 100 (see initICDC())*/
    fptr = fopen(newFileName, "w");
    checkFileOp(fptr, newFileName);
    fputs(headline, fptr);

    for (i = MEM_START_POINT; i < IC; i++) {
        convertBinToBase64(code64, i, FALSE); /* 3rd parameter is a boolean representing coding of data or instructions, see convertBinToBase64() */
        fputs(code64, fptr);
    }
    for (i = 0; i < DC; i++) {
        convertBinToBase64(code64, i, TRUE);
        fputs(code64, fptr);
    }
    fclose(fptr);
    checkFileOp(fptr, newFileName);
    free(newFileName);
}