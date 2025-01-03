#ifndef TLBLUR_H
#define TLBLUR_H

#include <sgx_trts.h>

int tlblur_enable(uint64_t vtlb_size);
uint64_t tlblur_disable();

#endif // TLBLUR_H
