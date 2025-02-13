#include "profiler.h"
#include "encl_u.h"
#include "pf.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/simstep.h"

void profiler_setup(int eid, int e_size, void *e_start, uint64_t argc, char **argv) {
}

void profiler_run(int eid) {
  // SGX_ASSERT(ecall_test(eid));
  int ret;
  uint64_t res[10];
  uint64_t data[10];
  uint64_t code[10];
  // SGX_ASSERT(ecall_select_benchmark(eid, 0));
  // SGX_ASSERT(ecall_run_benchmark(eid, &ret, 10, 10, res, data, code));
  // ASSERT(ret == 0);
  SGX_ASSERT(ecall_select_benchmark(eid, 1));
  start_single_stepping();
  SGX_ASSERT(ecall_run_benchmark(eid, &ret, 10, 10, res, data, code));
  stop_single_stepping();
}
