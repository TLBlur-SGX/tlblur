#include "encl_u.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/simstep.h"
#include "pf.h"
#include "profiler.h"

int exponent_d, cipher, plain;

void profiler_setup(int eid, int e_size, void *e_start, uint64_t argc,
                    char **argv) {
  if (argc < 1) {
    info("Invalid arguments for enclave call");
    abort();
  }
  exponent_d = atoi(argv[0]);
  void *sq_pt = NULL, *mul_pt = NULL, *modpow_pt = NULL, *stack_pt = NULL;
  SGX_ASSERT(ecall_get_square_adrs(eid, &sq_pt));
  SGX_ASSERT(ecall_get_multiply_adrs(eid, &mul_pt));
  SGX_ASSERT(ecall_get_modpow_adrs(eid, &modpow_pt));
  SGX_ASSERT(ecall_get_stack_adrs(eid, &stack_pt));
  modpow_pt = (void *)(((uint64_t)modpow_pt) & ~0xfff);
  info("stack somewhere around %p", stack_pt);
  info("square at %p; muliply at %p; modpow at %p", sq_pt, mul_pt, modpow_pt);

  SGX_ASSERT(ecall_rsa_set_d(eid, exponent_d));
  info("Secret value: %d", exponent_d);
  SGX_ASSERT(ecall_rsa_encode(eid, &cipher, 1234));

  info("RSA encode done: %d", cipher);
}

void profiler_run(int eid) {
  start_single_stepping();
  SGX_ASSERT(ecall_rsa_decode(eid, &plain, cipher));
  stop_single_stepping();

  info("RSA decode done: %d", plain);
}
