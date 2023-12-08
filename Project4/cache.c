#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

// **Note** this is a preprocessor macro. This is not the same as a function.
// Powers of 2 have exactly one 1 and the rest 0's, and 0 isn't a power of 2.
#define is_power_of_2(val) (val && !(val & (val - 1)))


/*
 * Accesses 1 word of memory.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads and 1 for writes.
 * write_data is a word, and is only valid if write_flag is 1.
 * If write flag is 1, mem_access does: state.mem[addr] = write_data.
 * The return of mem_access is state.mem[addr].
 */
extern int mem_access(int addr, int write_flag, int write_data);

/*
 * Returns the number of times mem_access has been called.
 */
extern int get_num_mem_accesses(void);

//Use this when calling printAction. Do not modify the enumerated type below.
enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

/* You may add or remove variables from these structs */
typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int tag;
    int valid;
} blockStruct;

//need an array of sets, each holding blocksPerSet blocks
typedef struct setStruct {
    blockStruct blocks[MAX_CACHE_SIZE];
} setStruct;

typedef struct cacheStruct
{
    setStruct sets[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache(void);
void updateLRUOnHit(int, int);
int addToCache(int, int, int);
/*
 * Set up the cache with given command line parameters. This is
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    if (blockSize <= 0 || numSets <= 0 || blocksPerSet <= 0) {
        printf("error: input parameters must be positive numbers\n");
        exit(1);
    }
    if (blocksPerSet * numSets > MAX_CACHE_SIZE) {
        printf("error: cache must be no larger than %d blocks\n", MAX_CACHE_SIZE);
        exit(1);
    }
    if (blockSize > MAX_BLOCK_SIZE) {
        printf("error: blocks must be no larger than %d words\n", MAX_BLOCK_SIZE);
        exit(1);
    }
    if (!is_power_of_2(blockSize)) {
        printf("warning: blockSize %d is not a power of 2\n", blockSize);
    }
    if (!is_power_of_2(numSets)) {
        printf("warning: numSets %d is not a power of 2\n", numSets);
    }
    printf("Simulating a cache with %d total lines; each line has %d words\n",
        numSets * blocksPerSet, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n",
        blocksPerSet, numSets);

    // Your code here

    //Need to set valid bits to 0, dirty to 0
    for (int j = 0; j < numSets; ++j) {
        
        for (int k = 0; k < blocksPerSet; ++k) {
            cache.sets[j].blocks[k].valid = 0;
            cache.sets[j].blocks[k].dirty = 0;
            cache.sets[j].blocks[k].lruLabel = 1000;
        }
    }
    cache.blockSize = blockSize;
    cache.blocksPerSet = blocksPerSet;
    cache.numSets = numSets;
    return;
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data)
{
    //First decode the address to find tag, set index, and block offset
    int numberSetBits = (int)log2(cache.numSets);
    int numberBlockBits = (int)log2(cache.blockSize);
 //   int numberTagBits = 16 - (numberSetBits + numberBlockBits);
    int ogAddr = addr;

    int blockMask = cache.blockSize - 1;
    int setMask = cache.numSets - 1;
    int blockBits = ogAddr & blockMask;

    ogAddr = ogAddr >> numberBlockBits;

    int setBits = ogAddr & setMask;

    ogAddr = ogAddr>> numberSetBits;

    int tag = ogAddr;
    int currentBlock = -1;
  //  act = memoryToCache;
    bool hit = false;
    //printf("Current Address: %d\nCurrent Write_flag: %d\n", addr, write_flag);
    //printCache();

    //Checks if the current data is in the cache
    for (int j = 0; j < cache.blocksPerSet; ++j) {
        
        //IF HIT
        if (tag == cache.sets[setBits].blocks[j].tag && cache.sets[setBits].blocks[j].valid) {
            hit = true;
            currentBlock = j;

        }

    }

    if (hit) {
        //Update LRU then execute instruction
        updateLRUOnHit(currentBlock, setBits);
        
    }
    else if (!hit) {
        //Add to cache, update LRU, execute instruction
        currentBlock = addToCache(tag, setBits, addr);
        if (currentBlock == -1) {
            printf("HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n\n\n\n\n\nHHHHHHHHH");
        }
    }

    //LW
    if (!write_flag) {
        printAction(addr, 1, cacheToProcessor);
        int currentData = cache.sets[setBits].blocks[currentBlock].data[blockBits];
        return currentData;
    }

    //SW
    if (write_flag) {
        printAction(addr, 1, processorToCache);
        cache.sets[setBits].blocks[currentBlock].dirty = 1;
        cache.sets[setBits].blocks[currentBlock].data[blockBits] = write_data;
    }

    return 0;
}

void updateLRUOnHit(int currentBlock, int currentSet) {

    for (int j = 0; j < cache.blocksPerSet; ++j) {
        if (cache.sets[currentSet].blocks[j].valid &&
            cache.sets[currentSet].blocks[j].lruLabel > cache.sets[currentSet].blocks[currentBlock].lruLabel) {
            cache.sets[currentSet].blocks[j].lruLabel--;
        }
    } 
    cache.sets[currentSet].blocks[currentBlock].lruLabel = cache.blocksPerSet-1 ;
    return;
}

int addToCache(int tagToAdd, int setToAddTo, int originalAddress) {

    //Convert tagToAdd into their multiple of blocksize
    originalAddress /= cache.blockSize;
    originalAddress *= cache.blockSize;

    setStruct setToSearch = cache.sets[setToAddTo];

    for (int j = 0; j < cache.blocksPerSet; ++j) {
        if (!(setToSearch.blocks[j].valid)) {
            //If this spot is open, add data to this block
            //Set this to LRU, decrement every other valid LRU

            //ADDING DATA TO BLOCK
            for (int k = 0; k < cache.blockSize; ++k) {
                cache.sets[setToAddTo].blocks[j].data[k] = mem_access(originalAddress + k, 0, 0);
            }

            //Set this as LRU
            cache.sets[setToAddTo].blocks[j].valid = 1;
            cache.sets[setToAddTo].blocks[j].dirty = 0;
            cache.sets[setToAddTo].blocks[j].lruLabel = cache.blocksPerSet - 1;
            cache.sets[setToAddTo].blocks[j].tag = tagToAdd;

            //Decrement every other block's LRU
            for (int k = 0; k < cache.blocksPerSet; ++k) {
                if ((k != j) && cache.sets[setToAddTo].blocks[k].valid) {
                    cache.sets[setToAddTo].blocks[k].lruLabel -= 1;
                }
            }
            printAction(originalAddress, cache.blockSize, memoryToCache);
            return j;
        }
        int addressToReturnTo = 0;
        if (setToSearch.blocks[j].lruLabel == 0)
        {
            //This is the least recently used, evict this and add block here
            //if (setToSearch.blocks[j].dirty) {


                addressToReturnTo += setToSearch.blocks[j].tag;
                addressToReturnTo = addressToReturnTo << (int)log2(cache.numSets);
                addressToReturnTo += setToAddTo;
                addressToReturnTo = addressToReturnTo << (int)log2(cache.blockSize);

                //Need to write this data back to memory
                if (setToSearch.blocks[j].dirty) {
                    for (int p = 0; p < cache.blockSize; ++p) {
                        mem_access(addressToReturnTo + p, 1, setToSearch.blocks[j].data[p]);
                    }
                }
                //Output that you moved it back to memory

                if (setToSearch.blocks[j].dirty) {
                    printAction(addressToReturnTo, cache.blockSize, cacheToMemory);
                }
                else {
                    printAction(addressToReturnTo, cache.blockSize, cacheToNowhere);
                }

                //Add the new block to cache and set LRU as necessary
                for (int k = 0; k < cache.blockSize; ++k) {
                    cache.sets[setToAddTo].blocks[j].data[k] = mem_access(originalAddress + k, 0, 0);
                }
                //Set this as LRU

                cache.sets[setToAddTo].blocks[j].valid = 1;
                cache.sets[setToAddTo].blocks[j].dirty = 0;
                cache.sets[setToAddTo].blocks[j].lruLabel = cache.blocksPerSet - 1;
                cache.sets[setToAddTo].blocks[j].tag = tagToAdd;

                //Decrement every other block's LRU
                for (int k = 0; k < cache.blocksPerSet; ++k) {
                    if ((k != j) && cache.sets[setToAddTo].blocks[k].valid) {
                        cache.sets[setToAddTo].blocks[k].lruLabel -= 1;
                    }
                }
                printAction(originalAddress, cache.blockSize, memoryToCache);

           // }
            
            

            return j;
        }

    }

    return -1;

}

/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(void)
{
    return;
}

/*
 * Log the specifics of each cache action.
 *
 *DO NOT modify the content below.
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
    else {
        printf("Error: unrecognized action\n");
        exit(1);
    }

}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache(void)
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                //printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
                printf(" %i", cache.sets[set].blocks[block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}
