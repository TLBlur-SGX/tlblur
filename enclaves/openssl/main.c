#include "libsgxstep/debug.h"
#include "encl_u.h"
#include <sgx_urts.h>

int main(int argc, char **argv) {
  sgx_launch_token_t token = {0};
  int updated = 0;
  sgx_enclave_id_t eid = -1;

  info_event("Creating enclave...");
  SGX_ASSERT(sgx_create_enclave(argv[1], /*debug=*/1, &token, &updated, &eid, NULL));
  int ret;
  uint64_t res[10];
  uint64_t data[10];
  uint64_t code[10];
  SGX_ASSERT(ecall_select_benchmark(eid, 0));
  SGX_ASSERT(ecall_run_benchmark(eid, &ret, 10, 10, res, data, code));
  ASSERT(ret == 0);
  SGX_ASSERT(ecall_select_benchmark(eid, 1));
  SGX_ASSERT(ecall_run_benchmark(eid, &ret, 10, 10, res, data, code));
  ASSERT(ret == 0);
  return 0;
}
