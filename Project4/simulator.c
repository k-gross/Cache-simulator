/*
 * Project 1
 * EECS 370 LC-2K Instruction-level simulator
 *
 * Make sure to NOT modify printState or any of the associated functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DO NOT CHANGE THE FOLLOWING DEFINITIONS 

// Machine Definitions
#define NUMMEMORY 65536 /* maximum number of words in memory (maximum number of lines in a given file)*/
#define NUMREGS 8 /*total number of machine registers [0,7]*/

// File Definitions
#define MAXLINELENGTH 1000 /* MAXLINELENGTH is the max number of characters we read */

typedef struct 
stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
static inline int convertNum(int32_t);
void add(stateType *);
void nor(stateType*);
void lw(stateType*);
void sw(stateType*);
void beq(stateType*);
void jalr(stateType*);
void halt(stateType*);

int main(int argc, char **argv)
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s , please ensure you are providing the correct path", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; ++state.numMemory) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address  %d\n. Please ensure you are providing a machine code file.", state.numMemory);
            perror("sscanf");
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    //Your code starts here!

    //Initialize values
    for (int j = 0; j < NUMREGS; ++j) {
        state.reg[j] = 0;
    }
    state.pc = 0;
    int count = 0;
    //Go through each line of memory
    while(state.pc < state.numMemory) {
        ++count;
        //Print initial state
        printState(&state);

        //Find which command you are decoding, find opcode in 24-22
        int opcode = state.mem[state.pc] >> 22;
        if (opcode == 0) {
            add(&state);
            state.pc = state.pc + 1;

        }
        else if (opcode == 1) {
            nor(&state);
            state.pc = state.pc + 1;

        }
        else if (opcode == 2) {
            lw(&state);
            state.pc = state.pc + 1;

        }
        else if (opcode == 3) {
            sw(&state);
            state.pc = state.pc + 1;

        }
        else if (opcode == 4) {
            beq(&state);
        }
        else if (opcode == 5) {
            jalr(&state);
        }
        else if (opcode == 6) {
            state.pc = state.pc + 1;
            break;
        }
        else if (opcode == 7) {
            //Do nothing
            state.pc = state.pc + 1;
        }
       


    }
    printf("machine halted\ntotal of %d instructions executed\nfinal state of machine : \n", count);
    printState(&state);


    //Your code ends here! 

    return(0);
}

void add(stateType * state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;
    int destReg = (state->mem[state->pc]) & 7;

    //add contents of regA with contents of regB, store results in destReg
    state->reg[destReg] = state->reg[regA] + state->reg[regB];
}

void nor(stateType* state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;
    int destReg = (state->mem[state->pc]) & 7;

    //Nor contents of regA with contents of regB, store results in destReg
    state->reg[destReg] = ~(state->reg[regA] | state->reg[regB]);
}

void lw(stateType* state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;

    int offset = (state->mem[state->pc]) & 0x0000FFFF;
    offset = convertNum(offset);
  
    //Load regB from memory. Memory address is formed by adding offset with regA.
    state->reg[regB] = state->mem[offset + state->reg[regA]];
}

void sw(stateType* state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;

    int offset = (state->mem[state->pc]) & 0x0000FFFF;
    offset = convertNum(offset);

    //Load regB from memory. Memory address is formed by adding offset with regA.
    state->mem[offset + state->reg[regA]] = state->reg[regB];
}

void beq(stateType* state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;

    int offset = (state->mem[state->pc]) & 0x0000FFFF;
    offset = convertNum(offset);

    //If regB and regA contents are equal, brach to PC+1+Offset
    if (state->reg[regA] == state->reg[regB]) {
        state->pc = state-> pc + 1 + offset;
    }
    else {
        state->pc = state->pc + 1;
    }
}

void jalr(stateType* state) {
    int regA = (state->mem[state->pc] >> 19) & 7;
    int regB = (state->mem[state->pc] >> 16) & 7;

    int offset = (state->mem[state->pc]) & 0x0000FFFF;
    offset = convertNum(offset);
    
    //First store PC+1 into regB
    state->reg[regB] = state->pc + 1;

    //Then branch to address in regA
    state->pc = state->reg[regA];
}



/*
* DO NOT MODIFY ANY OF THE CODE BELOW. 
*/

void 
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) 
              printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) 
              printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    printf("end state\n");
}

// convert a 16-bit number into a 32-bit Linux integer
static inline int convertNum(int num) 
{
    return num - ( (num & (1<<15)) ? 1<<16 : 0 );
}

/*
* Write any helper functions that you wish down here. 
*/