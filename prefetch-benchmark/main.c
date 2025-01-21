/*
 *  This file is part of the SGX-Step enclave execution control framework.
 *
 *  Copyright (C) 2017 Jo Van Bulck <jo.vanbulck@cs.kuleuven.be>,
 *                     Raoul Strackx <raoul.strackx@cs.kuleuven.be>
 *
 *  SGX-Step is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SGX-Step is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SGX-Step. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sgx_urts.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <ucontext.h>

#include "Enclave/encl_u.h"
#include "libsgxstep/cpu.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/elf_parser.h"
#include "libsgxstep/enclave.h"
#include "libsgxstep/pt.h"

#define DBG_ENCL 1
#define ENCLAVE_SO "./Enclave/encl.so"
#define MAX_MEASUREMENTS 1000000

sgx_enclave_id_t eid = 0;
int do_measurement = 0;
uint64_t time_begin = 0;
uint64_t time_end = 0;
uint64_t timings[16];
int count = 0;

uint64_t current_index = 0;

typedef struct measurement {
  uint64_t total;
  uint64_t base;
  uint64_t sort;
  uint64_t prefetch;
} measurement_t;
measurement_t measurements[MAX_MEASUREMENTS] = {0};

void ocall_print_string(const char *s) {
  printf("%s", s);
  fflush(stdout);
}

void ocall_print_int(const char *str, uint64_t i) {
  printf(str, i);
  fflush(stdout);
}

void aep_cb_func(void) {
  if (!do_measurement)
    return;
  time_begin = rdtsc_begin();
  do_measurement = 0;
}

void fault_handler(int signo, siginfo_t *si, void *ctx) {
  switch (signo) {
  case SIGILL:
    if (time_end != 0 && time_begin != 0) {
      measurement_t *m = &measurements[current_index++ % MAX_MEASUREMENTS];
      m->total = time_end - time_begin;
      m->base = timings[0] - time_begin;
      m->sort = timings[1] - timings[0];
      m->prefetch = time_end - timings[2];
    }

    do_measurement = 1;
    break;

  default:
    info("Caught unknown signal '%d'", signo);
    abort();
  }
}

int main(int argc, char **argv) {
  struct sigaction act, old_act;
  sgx_launch_token_t token = {0};
  int updated = 0;

  if (argc < 3) {
    printf("Usage: ./app <name> <num measurements> <pws size>\n");
    exit(1);
  }
  uint64_t num = atoi(argv[2]);
  ASSERT(num < MAX_MEASUREMENTS);
  uint64_t pws_size = atoi(argv[3]);

  info("Creating enclave...");
  SGX_ASSERT(
      sgx_create_enclave(ENCLAVE_SO, DBG_ENCL, &token, &updated, &eid, NULL));

  memset(&act, 0, sizeof(sigaction));
  act.sa_sigaction = fault_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  sigfillset(&act.sa_mask);
  ASSERT(!sigaction(SIGILL, &act, &old_act));

  register_symbols(ENCLAVE_SO);
  register_aep_cb(aep_cb_func);
  print_enclave_info();

  uint64_t pam_size;
  SGX_ASSERT(enclave_setup(eid, &pam_size, pws_size, timings));
  char filename[200];
  snprintf(filename, 200, "%s_n%lu_pws%lu_pam%lu.csv", argv[1], num, pws_size,
           pam_size);

  info_event("calling enclave..");
  SGX_ASSERT(enclave_call(eid, &time_end, num));

  FILE *file = fopen(filename, "w");
  fprintf(file, "total,base,sort,prefetch\n");
  for (int i = (num / 4); i < num; i++) {
    measurement_t m = measurements[i];
    if (m.total == 0)
      break;
    fprintf(file, "%d,%d,%d,%d\n", m.total, m.base, m.sort, m.prefetch);
  }

  info("all is well; exiting..");
  SGX_ASSERT(sgx_destroy_enclave(eid));
  return 0;
}
