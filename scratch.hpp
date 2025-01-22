#ifndef _SCRATCH_MEMORY_HPP
#define _SCRATCH_MEMORY_HPP

void init_scratch(void);
void *scratch_alloc(unsigned int size);
void *scratch_calloc(unsigned int count, unsigned int size);

#define ALLOC_STRUCT_SCRATCH(Type) (Type*) scratch_alloc(sizeof(Type))
#define ALLOC_ARRAY_SCRATCH(amount, Type) (Type*) scratch_calloc(amount, sizeof(Type))

#endif