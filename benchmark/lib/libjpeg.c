#include "encl_u.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/file.h"
#include "pf.h"
#include "sgx_error.h"
#include <fcntl.h>
#include <sgx_urts.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "benchmark.h"

int tlblur_benchmark_setup(sgx_enclave_id_t eid, uint64_t argc, char **argv) {
  if (argc != 3)
    return 1;
  char *image_path = argv[0];
  size_t buffer_size = atoi(argv[1]);
  size_t max_size = atoi(argv[2]);
  printf("input size: %d, output_size: %d\n", buffer_size, max_size);
  uint8_t *in_buffer = malloc(buffer_size);
  ASSERT(in_buffer);
  size_t in_sz = file_read(image_path, in_buffer, buffer_size);
  info("loading image into enclave");
  int load_res;
  sgx_status_t res = enclave_jpeg_load_image(eid, &load_res, in_buffer, in_sz, max_size);
  if (res != SGX_SUCCESS)
    return res;
  info("loaded image into enclave");
  free(in_buffer);
  return load_res;
}

int tlblur_benchmark_destroy(sgx_enclave_id_t eid) {
  info("freeing image");
  sgx_status_t res = enclave_jpeg_free_image(eid);
  if (res != SGX_SUCCESS)
    return res;
  return 0;
}
