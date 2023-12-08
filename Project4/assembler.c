/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static inline int isNumber(char *);

void errorCheck(char* field0, char* field1, char* field2) {
    if (atoi(field0) > 7 || atoi(field0) < 0 || atoi(field1) > 7 ||
        atoi(field1) < 0 || atoi(field2) > 7 || atoi(field2) < 0) {
        exit(1);
    }
    //Check if registers are non integers
    if (isalpha(field0[0]) || isalpha(field1[0]) || isalpha(field2[0])) {
        exit(1);
    }

    return;
}

void errorCheck2(char* field0, char* field1) {
    if (atoi(field0) > 7 || atoi(field0) < 0 || atoi(field1) > 7 ||
        atoi(field1) < 0) {
        exit(1);
    }
    //Check if registers are non integers
    if (isalpha(field0[0]) || isalpha(field1[0])) {
        exit(1);
    }

    return;
}


void checkOffset(int offset) {
    if (offset > 32767 || offset < -32768) {
        exit(1);
    }
    return;

}
void checkDup(char(*ptr)[7], char* val, int limit) {
    
    for (int i = 0; i < limit; ++i) {
        if (!strcmp(ptr[i], val)) {
            exit(1);
        }
    }
    return;
}

int address(char (*ptr)[7], char* val, int limit) {

    for (int i = 0; i < limit; ++i) {
        if (!strcmp(ptr[i], val)) {
            return i;
        }
    }
    exit(1);
}

int add(char* field0, char* field1, char* field2) {
    errorCheck(field0, field1, field2);
    int bits = 0;

    //add opcode
    bits += (0 << 22);

    //add field0
    int cur = atoi(field0);
    bits += (cur << 19);

    //field1
    cur = atoi(field1);
    bits += (cur << 16);

    cur = atoi(field2);
    bits += cur;
    
    return bits;
}

int nor(char* field0, char* field1, char* field2) {
    errorCheck(field0, field1, field2);
    int bits = 0;

    //opcode
    bits += (1 << 22);

    int cur = atoi(field0);
    bits += (cur << 19);

    cur = atoi(field1);
    bits += (cur << 16);

    cur = atoi(field2);
    bits += cur;

    return bits;
}

int lw(char* field0, char* field1, char* field2, char (*ptr)[7], int max) {
    errorCheck2(field0, field1);
    int bits = 0;
    

    bits += (2 << 22);

    int cur = atoi(field0);
    bits += (cur << 19);

    cur = atoi(field1);
    bits += (cur << 16);

    if (isalpha(field2[0])) {
        //find address of label and add
        int huh = address(ptr, field2, max);

        bits += (huh & 0x0000FFFF);
    }
    else {
        cur = atoi(field2);
        //Check if val too big
        checkOffset(cur);
        bits += (cur & 0x0000FFFF);
    }

    return bits;
}

int sw(char* field0, char* field1, char* field2, char(*ptr)[7], int max) {
    errorCheck2(field0, field1);
    int bits = 0;

    bits += (3 << 22);

    int cur = atoi(field0);
    bits += (cur << 19);

    cur = atoi(field1);
    bits += (cur << 16);

    if (isalpha(field2[0])) {
        //find address of label and add
        int huh = address(ptr, field2, max);

        bits += (0x0000FFFF & huh);
    }
    else {
        cur = atoi(field2);

        //Check val too big
        checkOffset(cur);
        bits += (0x0000FFFF & cur);
    }
    return bits;
}

int beq(int count, char* field0, char* field1, char* field2, char(*ptr)[7], int max) {
    errorCheck2(field0, field1);
    int bits = 0;
    bits += (4 << 22);

    int cur = atoi(field0);
    bits += (cur << 19);

    cur = atoi(field1);
    bits += (cur << 16);

    //Decide how to calculate offset field
    if (isalpha(field2[0])) {
        //Find address of label and calculate based off of that
        int huh = address(ptr, field2, max);
        int offset = huh - (count + 1);
        offset = offset & 0x0000FFFF;

        bits += offset;
    }
    else {
        cur = atoi(field2);
        //Check if val too big
        checkOffset(cur);

        bits += (cur & 0x0000FFFF);
    }
    return bits;
}

int jalr(char* field0, char* field1) {
    errorCheck2(field0, field1);
    int bits = 0;

    bits += (5 << 22);

    int cur = atoi(field0);
    bits += (cur << 19);

    cur = atoi(field1);
    bits += (cur << 16);

    return bits;
}

int fill(char* field0, char (*ptr)[7], int max) {

    if (isalpha(field0[0])) {
        return address(ptr, field0, max);
    }
    int bits = atoi(field0);
    return bits;
}



int main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        /* reached end of file */
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);
    char labelAddress[65536][7];
    //char(*gosh)[7] = labelAddress;
    int pc = 0;
    int programLength = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {
        
        if (strlen(label) > 0)
        {
            strcpy(labelAddress[pc], label);
            checkDup(labelAddress, labelAddress[pc], pc);
        }
        else {
            strcpy(labelAddress[pc], "");
        }
        ++pc;
    }
    programLength = pc;
    rewind(inFilePtr);

    int test;
    pc = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) {

        //Check to make sure opcode and arguments are bound to have errors
        //errorCheck(arg0, arg1, arg2);
        
        if (!strcmp(opcode, "add")) {
            test = add(arg0, arg1, arg2);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "nor")) {
            test = nor(arg0, arg1, arg2);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "lw")) {
            test = lw(arg0, arg1, arg2, labelAddress, programLength);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "sw")) {
            test = sw(arg0, arg1, arg2, labelAddress, programLength);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "beq")) {
            test = beq(pc, arg0, arg1, arg2, labelAddress, programLength);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "jalr")) {
            test = jalr(arg0, arg1);
            fprintf(outFilePtr, "%d\n", test);
        }
        else if (!strcmp(opcode, "halt")) {
            fprintf(outFilePtr, "%d\n", 25165824);
        }
        else if (!strcmp(opcode, "noop")) {
            fprintf(outFilePtr, "%d\n", 29360128);
        }
        else if (!strcmp(opcode, ".fill")) {
            test = fill(arg0, labelAddress, programLength);
            fprintf(outFilePtr, "%d\n", test);
        }
        else {
            //Error code for when no opcodes match
            exit(1);
        }
        
        ++pc;
        

    }
    /* after doing a readAndParse, you may want to do the following to test the
        opcode */
    
    exit(0);
    return(0);
}

/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Treat a blank line as end of file.
    // Arguably, we could just ignore and continue, but that could
    // get messy in terms of label line numbers etc.
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(size_t line_idx = 0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                ++line_char_is_whitespace;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            ++nonempty_line;
            break;
        }
    }
    if(nonempty_line == 0) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}

