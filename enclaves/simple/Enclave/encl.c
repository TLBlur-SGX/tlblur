#include "encl_t.h"
#include "sgx_trts_aex.h"
#include <sgx_trts.h>

/* see asm.S */
extern int aaaaaa;
extern char _srx;
extern char _erx;
extern char _srx1;
extern char _erx1;
extern char _ero;
extern char _sro;
extern char _erw;
extern char _srw;
extern char __tlblur_endtext;
extern char __ImageBase;


void ecall_inc_secret(int s)
{
    tlblur_enable(50);
    if (s)
        aaaaaa += 1;
    tlblur_disable();
}

void ecall_no_inc_secret(int s)
{
    return;
}

void *ecall_get_a_adrs(void)
{
    return (void*) &aaaaaa;
}

int benchmark_inner() {
    return 0;
}
