#include "cacheutils.h"
#include <stdint.h>

extern uint64_t __tlblur_shadow_pt;
extern uint64_t __tlblur_global_counter;
extern uint64_t __tlblur_global_code_counter;
extern void tlblur_tlb_update(void);
extern int benchmark_inner();

char *ecall_get_vtlb_addr() { return (char *)(&__tlblur_shadow_pt); }

void *ecall_get_tlblur_code_addr() { return (void *)(tlblur_tlb_update); }

uint64_t *ecall_get_vtlb_counter() { return &__tlblur_global_counter; }

int ecall_run_benchmark(uint64_t warmup_iterations, uint64_t iterations,
                        uint64_t *results, uint64_t *data_accesses,
                        uint64_t *code_accesses) {
  for (int i = 0; i < warmup_iterations; i++) {
    int res = benchmark_inner();
    if (res != 0)
      return res;
  }
  
  for (int i = 0; i < iterations; i++) {
    uint64_t data_counter_original = __tlblur_global_counter;
    uint64_t code_counter_original = __tlblur_global_code_counter;

    uint64_t counter_start = rdtsc_begin();
    int res = benchmark_inner();
    if (res != 0)
      return res;
    results[i] = rdtsc_end() - counter_start;

    data_accesses[i] = __tlblur_global_counter - data_counter_original;
    code_accesses[i] = __tlblur_global_code_counter - code_counter_original;
  }

  return 0;
}
