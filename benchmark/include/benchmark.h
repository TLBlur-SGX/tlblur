#include "sgx_edger8r.h"
#include <stdint.h>

int tlblur_benchmark_setup(sgx_enclave_id_t eid, uint64_t argc, char **argv);
int tlblur_benchmark_run(sgx_enclave_id_t eid, uint64_t iterations,
                         int external_loop, uint64_t do_warmup, uint64_t *results,
                         uint64_t *data_accesses, uint64_t *code_accesses);
int tlblur_benchmark_destroy(sgx_enclave_id_t eid);
