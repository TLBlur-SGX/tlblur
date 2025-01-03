#include "encl_u.h"
#include "benchmark.h"
#include "libsgxstep/debug.h"
#include "sgx_urts.h"

int tlblur_benchmark_setup(sgx_enclave_id_t eid, uint64_t argc, char **argv) {
  if (argc != 1)
    return 1;

  uint64_t benchmark_id = atoi(argv[0]);
  printf("selecting benchmark %d\n", benchmark_id);
  sgx_status_t res = ecall_select_benchmark(eid, benchmark_id);
  if (res != SGX_SUCCESS)
    return res;
  return 0;
}

int tlblur_benchmark_destroy(sgx_enclave_id_t eid) {
  return 0;
}
