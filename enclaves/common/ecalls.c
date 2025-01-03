#include "sgx_edger8r.h"
#include "libsgxstep/debug.h"
#include <stdint.h>
#include <stdio.h>

sgx_status_t ecall_get_vtlb_addr(sgx_enclave_id_t eid, char **retval);
sgx_status_t ecall_get_vtlb_counter(sgx_enclave_id_t eid, uint64_t **retval);
sgx_status_t ecall_get_tlblur_code_addr(sgx_enclave_id_t eid, void **retval);

char *tlblur_get_vtlb_addr(sgx_enclave_id_t eid) {
  char *value;
  SGX_ASSERT(ecall_get_vtlb_addr(eid, &value));
  printf("vtlb: %p\n", value);
  return value;
}

uint64_t *tlblur_get_vtlb_counter(sgx_enclave_id_t eid) {
  uint64_t *value;
  SGX_ASSERT(ecall_get_vtlb_counter(eid, &value));
  printf("vtlb_counter: %p\n", value);
  return value;
}

void *tlblur_get_code_addr(sgx_enclave_id_t eid) {
  void *value;
  SGX_ASSERT(ecall_get_tlblur_code_addr(eid, &value));
  printf("%p\n", value);
  return value;
}
