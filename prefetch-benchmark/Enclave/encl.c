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

#include "encl_t.h"
#include "sgx_trts_aex.h"
#include "sgx_trts_exception.h"

extern void tlblur_tlb_update(uint64_t addr);
extern char dummy_array[10000000];
extern uint64_t g_tlblur_pam_size;
uint64_t dummy[16] = {69};
uint64_t *timings = dummy;

int skip_ud2(sgx_exception_info_t *info) {
  if (info->exception_vector == SGX_EXCEPTION_VECTOR_UD) {
    info->cpu_context.rip += 2;
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  return EXCEPTION_CONTINUE_SEARCH;
}

uint64_t enclave_setup(uint64_t pws, uint64_t *t) {
  timings = t;
  sgx_register_exception_handler(1, skip_ud2);

  for (int i = 0; i < 500; i++) {
    tlblur_tlb_update((uint64_t)(dummy_array + (i * 4096)));
  }

  tlblur_enable(pws);
  return g_tlblur_pam_size;
}
