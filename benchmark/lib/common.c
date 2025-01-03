#include "benchmark.h"
#include "libsgxstep/cpu.h"
#include "libsgxstep/debug.h"
#include <sgx_urts.h>

extern sgx_status_t ecall_run_benchmark(sgx_enclave_id_t eid, int *retval,
                                        uint64_t warmup_iterations,
                                        uint64_t iterations, uint64_t *results,
                                        uint64_t *data_accesses,
                                        uint64_t *code_accesses);

int tlblur_benchmark_run(sgx_enclave_id_t eid, uint64_t iterations,
                         int external_loop, uint64_t warmup, uint64_t *results,
                         uint64_t *data_accesses, uint64_t *code_accesses) {
  info("running benchmark");
  uint64_t internal_iterations = external_loop ? 1 : iterations;
  uint64_t external_iterations = external_loop ? iterations : 1;
  printf("internal iterations: %d, external iterations: %d\n",
         internal_iterations, external_iterations);

  if (external_loop) {
    uint64_t result;
    uint64_t data;
    uint64_t code;
    for (int i = 0; i < warmup; i++) {
      info("warmup");
      int benchmark_res;
      sgx_status_t res = ecall_run_benchmark(
          eid, &benchmark_res, 0, 1, &result, &data, &code);
      if (res != SGX_SUCCESS)
        return res;
      if (benchmark_res != 0)
        return benchmark_res;
    }
  }

  for (int i = 0; i < external_iterations; i++) {
    uint64_t counter_start = rdtsc_begin();

    int benchmark_res;
    sgx_status_t res = ecall_run_benchmark(
        eid, &benchmark_res, external_loop ? 0 : warmup, internal_iterations,
        results + i, data_accesses + i, code_accesses + i);

    if (external_loop) {
      results[i] = rdtsc_end() - counter_start;
    }
    
    if (res != SGX_SUCCESS)
      return res;
    if (benchmark_res != 0)
      return benchmark_res;
  }

  return 0;
}
