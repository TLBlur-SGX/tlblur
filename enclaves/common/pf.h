#ifndef PF_H_INC
#define PF_H_INC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PFN_MASK 0xfff 

#define GET_PFN(adrs) ((void*) (((uint64_t) adrs) & ~PFN_MASK))

#define PROT_NONE_SHIFT			8
#define PROT_NONE_MASK			(UINT64_C(0x1) << PROT_NONE_SHIFT)

#define _PROT_NONE(entry) 		(((entry) & PROT_NONE_MASK) >> PROT_NONE_SHIFT)
#define MARK_PROT_NONE(entry) 		((entry) | PROT_NONE_MASK)
#define MARK_NOT_PROT_NONE(entry) 	((entry) & ~PROT_NONE_MASK)

typedef void (*fault_handler_t)(void *page_base_adrs);
void register_fault_handler(fault_handler_t cb);

#endif
