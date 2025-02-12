// #include "cacheutils.h"
#include "encl_t.h"
#include "tlblur.h"
#include <sgx_trts.h>
#include <stdint.h>

#define SIZE 0x10000

void long_func(void);
void long_loop(void);
uint64_t memory[SIZE] = {0};
uint64_t benchmark_id = 0;

void stressmem() {
  for (int i = 0; i < SIZE; i += 0x1) {
    memory[i] = 42069;
  }

  volatile uint64_t read = 0;
  for (int i = 0; i < SIZE; i += 0x1) {
    read = memory[i];
  }
}

int fibonacci(int n) {
  if (n <= 1)
    return n;
  else
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void ecall_select_benchmark(uint64_t id) {
  benchmark_id = id;
}

int benchmark_inner() {
  switch (benchmark_id) {
  case 1:
    stressmem();
    break;
  case 2:
    fibonacci(10);
    break;
  case 3:
    long_func();
    break;
  case 4:
    long_loop();
    break;
  }
  return 0;
}

