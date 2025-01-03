/* utility headers */
#include "cacheutils.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/enclave.h"
#include "pf.h"
#include <sys/mman.h>

#include "encl_u.h"
#include <sgx_urts.h>

#define NUM_ITERATIONS 200000

sgx_enclave_id_t create_enclave(char *so) {
  sgx_launch_token_t token = {0};
  int updated = 0;
  sgx_enclave_id_t eid = -1;

  info_event("Creating enclave...");
  SGX_ASSERT(sgx_create_enclave(so, /*debug=*/1, &token, &updated, &eid, NULL));

  return eid;
}

void write_results(uint64_t *results) {
  FILE *file = fopen("output.txt", "w");

  for (int i = 0; i < NUM_ITERATIONS; i++) {
    fprintf(file, "%d\n", results[i]);
  }

  fclose(file);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    info("Invalid arguments");
    exit(1);
  }

  sgx_enclave_id_t eid = create_enclave(argv[1]);

  uint64_t results[NUM_ITERATIONS];
  uint64_t data_accesses = 0;
  uint64_t code_accesses = 0;
  int ret = -1;
  ecall_select_benchmark(eid, 3);
  ecall_run_benchmark(eid, &ret, 0, NUM_ITERATIONS, results, &data_accesses,
                      &code_accesses);
  printf("Number of instrumentation calls: data: %d, code: %d\n", data_accesses,
         code_accesses);
  write_results(results);

  info_event("destroying SGX enclave");
  SGX_ASSERT(sgx_destroy_enclave(eid));

  info("all is well; exiting..");
  return 0;
}
