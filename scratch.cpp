#include "scratch.hpp"
#include <string.h>
#include <stdio.h>

#define SCRATCH_SIZE 4096 * 4096
unsigned char scratch_memory[SCRATCH_SIZE];
unsigned int used;

#ifndef EDITOR_DEBUG
#define _ASSERT(E,m) {}
#else
#define _ASSERT(E,m) if (!(E)) { printf("%s\n", m); *((int*)0) = 0;}
#endif

void init_scratch(void) 
{ 
    used = 0;
#ifdef EDITOR_DEBUG
    memset(scratch_memory, 0, SCRATCH_SIZE);
#endif
}

void* scratch_alloc(unsigned int size) 
{
    void* result = 0;
    _ASSERT(used + size < SCRATCH_SIZE, "OUT OF SCRATCH MEMORY");
    
    if(used + size < SCRATCH_SIZE) 
    {
        result = scratch_memory + used;
        used += size;
    }
    return result;
}

void *scratch_calloc(unsigned int count, unsigned int size)
{
    return scratch_alloc(count * size);
}


