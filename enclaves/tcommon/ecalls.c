#include "cacheutils.h"
#include <stdint.h>

extern uint64_t __tlblur_pam;
extern uint64_t __tlblur_counter;
extern void tlblur_pam_update(void);
extern int benchmark_inner();

char *ecall_get_vtlb_addr() { return (char *)(&__tlblur_pam); }

void *ecall_get_tlblur_code_addr() { return (void *)(tlblur_pam_update); }

uint64_t *ecall_get_vtlb_counter() { return &__tlblur_counter; }

int ecall_run_benchmark(uint64_t warmup_iterations, uint64_t iterations,
                        uint64_t *results, uint64_t *data_accesses,
                        uint64_t *code_accesses) {
  for (int i = 0; i < warmup_iterations; i++) {
    int res = benchmark_inner();
    if (res != 0)
      return res;
  }
  
  for (int i = 0; i < iterations; i++) {
    uint64_t counter_original = __tlblur_counter;

    uint64_t counter_start = rdtsc_begin();
    int res = benchmark_inner();
    if (res != 0)
      return res;
    results[i] = rdtsc_end() - counter_start;

    data_accesses[i] = __tlblur_counter - counter_original;
  }

  return 0;
}
