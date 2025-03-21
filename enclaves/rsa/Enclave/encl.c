#include "encl_t.h"
#include "tlblur.h"
// #include <sgx_trts.h>
// #include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Compute n^-1 mod m by extended euclidian method.
 * (from https://github.com/pantaloons/RSA/blob/master/single.c)
 */
int inverse(int n, int modulus)
{
	int a = n, b = modulus;
	int x = 0, y = 1, x0 = 1, y0 = 0, q, temp;
	while (b != 0)
	{
		q = a / b;
		temp = a % b;
		a = b;
		b = temp;
		temp = x; x = x0 - q * x; x0 = temp;
		temp = y; y = y0 - q * y; y0 = temp;
	}
	if (x0 < 0) x0 += modulus;
	return x0;
}

/*
 * NOTE: we provide a page-aligned assembly version of this function in asm.S
 * to ease the attack.
 */
long long c_square(long long a, long long n)
{
    return (a * a) % n;
}

/*
 * NOTE: we provide a page-aligned assembly version of this function in asm.S
 * to ease the attack.
 */
long long c_multiply(long long a, long long b, long long n)
{
    return (a * b) % n;
}

/* See asm.S */
uint64_t square(uint64_t a, uint64_t n);
uint64_t multiply(uint64_t a, uint64_t b, uint64_t n);

/*
 * Computes a^b mod n.
 * (long long multiplication prevents overflow)
 */
int modpow(long long a, long long b, long long n)
{
    // volatile int zero = atoi("0");
    // malloc(0);
    char text[50];
    strncpy(text, "hello", 50);
    long long res = 1;
    uint16_t mask = 0x8000;

    for (int i=15; i >= 0; i--)
    {
        res = square(res, n);
        if (b & mask) {
            res = multiply(res, a, n); 
        }
        mask = mask >> 1;
    }
    return res;
}

/*
 * Example RSA key with p = 137 and q = 421.
 */
int rsa_n = 57677;
int rsa_e = 11;
int rsa_d = 20771; //16'b0101000100100011

void ecall_rsa_set_d(int d) {
    rsa_d = d;
}

int ecall_rsa_decode(int cipher)
{
    tlblur_enable(5);
    int i, mask, r = 0;
    long long res;
    
    /* Blinding with 16-bit random factor. */
    if (sgx_read_rand((unsigned char*) &r, 2)
            != SGX_SUCCESS) return 0;
    cipher = cipher * modpow(r, rsa_e, rsa_n);
    
    /* Decrypt blinded message with square and multiply algorithm. */
    res = modpow(cipher, rsa_d, rsa_n);

    tlblur_disable();
    return (res * inverse(r, rsa_n)) % rsa_n;
}

int ecall_rsa_encode(int plain)
{
    return modpow(plain, rsa_e, rsa_n);
}

void *ecall_get_square_adrs(void)
{
    return square;    
}

void *ecall_get_multiply_adrs(void)
{
    return multiply;
}

void *ecall_get_modpow_adrs(void)
{
    return modpow;
}

void *ecall_get_stack_adrs(void)
{
    int a = 42;
    return &a;
}

int benchmark_inner() {
    return 0;
}
