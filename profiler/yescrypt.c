#include "profiler.h"
#include "encl_u.h"
#include "pf.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/simstep.h"

void profiler_setup(int eid, int e_size, void *e_start, uint64_t argc, char **argv) {
  sgx_status_t res = ecall_select_benchmark(eid, atoi(argv[0]));
}

void profiler_run(int eid) {
  int ret;
  uint64_t result;
  uint64_t code;
  uint64_t data;
  start_single_stepping();
  ecall_run_benchmark(eid, &ret, 0, 1, &result, &data, &code);
  stop_single_stepping();
  printf("Done: %d (cycles = %d, code = %d, data = %d)", ret, result, code, data);
}
