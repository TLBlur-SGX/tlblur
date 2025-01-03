#ifndef CACHE_UTILS_H_INC
#define CACHE_UTILS_H_INC

#include <stdint.h>

/*
 * Code adapted from
 * https://github.com/IAIK/flush_flush/blob/master/sc/cacheutils.h
 */
inline uint64_t rdtsc_begin( void )
{
  uint64_t begin;
  uint32_t a, d;

  asm volatile (
    "mfence\n\t"
    "RDTSCP\n\t"
    "mov %%edx, %0\n\t"
    "mov %%eax, %1\n\t"
    "mfence\n\t"
    : "=r" (d), "=r" (a)
    :
    : "%eax", "%ebx", "%ecx", "%edx");

  begin = ((uint64_t)d << 32) | a;
  return begin;
}

inline uint64_t rdtsc_end( void )
{
  uint64_t end;
  uint32_t a, d;

  asm volatile(
    "mfence\n\t"
    "RDTSCP\n\t"
    "mov %%edx, %0\n\t"
    "mov %%eax, %1\n\t"
    "mfence\n\t"
    : "=r" (d), "=r" (a)
    :
    : "%eax", "%ebx", "%ecx", "%edx");

  end = ((uint64_t)d << 32) | a;
  return end;
}

#endif
