#include "encl_t.h"
#include "tlblur.h"
#include "wolfssl/wolfcrypt/asn.h"
#include "wolfssl/wolfcrypt/error-crypt.h"
#include "wolfssl/wolfcrypt/memory.h"
#include "wolfssl/wolfcrypt/random.h"
#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/types.h"
#include "wolfssl/wolfcrypt/wc_port.h"
#include "wolfssl/wolfcrypt/wolfmath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wolfssl/certs_test.h>
#include <wolfssl/wolfcrypt/aes.h>
#include <wolfssl/wolfcrypt/rsa.h>

void printf(const char *fmt, ...) {
  char buf[BUFSIZ] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  ocall_print_string(buf);
}

int sprintf(char *buf, const char *fmt, ...) {
  va_list ap;
  int ret;
  va_start(ap, fmt);
  ret = vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  return ret;
}

double current_time(void) {
  double curr;
  ocall_current_time(&curr);
  return curr;
}

int LowResTimer(void) /* low_res timer */
{
  int time;
  ocall_low_res_time(&time);
  return time;
}

size_t recv(int sockfd, void *buf, size_t len, int flags) {
  size_t ret;
  int sgxStatus;
  sgxStatus = ocall_recv(&ret, sockfd, buf, len, flags);
  return ret;
}

size_t send(int sockfd, const void *buf, size_t len, int flags) {
  size_t ret;
  int sgxStatus;
  sgxStatus = ocall_send(&ret, sockfd, buf, len, flags);
  return ret;
}

static THREAD_LS_T byte *bench_plain = NULL;
static THREAD_LS_T byte *bench_cipher = NULL;
#ifndef NO_FILESYSTEM
static THREAD_LS_T char *hash_input = NULL;
static THREAD_LS_T char *cipher_input = NULL;
#endif

static const XGEN_ALIGN byte bench_key_buf[] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xde, 0xba,
    0x98, 0x76, 0x54, 0x32, 0x10, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23,
    0x45, 0x67, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xf0,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb,
    0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

static const XGEN_ALIGN byte bench_iv_buf[] = {
    0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x81};
static THREAD_LS_T byte *bench_key = NULL;
static THREAD_LS_T byte *bench_iv = NULL;

#define BENCH_MAX_PENDING 1

#ifdef WOLFSSL_STATIC_MEMORY
static WOLFSSL_HEAP_HINT *HEAP_HINT;
#else
#define HEAP_HINT NULL
#endif

#ifdef WOLFSSL_CAAM
#include <wolfssl/wolfcrypt/port/caam/wolfcaam.h>
#ifdef WOLFSSL_SECO_CAAM
#define SECO_MAX_UPDATES 10000
#define SECO_BENCHMARK_NONCE 0x7777
#define SECO_KEY_STORE_ID 1
#endif

static THREAD_LS_T int devId = WOLFSSL_CAAM_DEVID;
#else
#ifdef WC_USE_DEVID
static THREAD_LS_T int devId = WC_USE_DEVID;
#else
static THREAD_LS_T int devId = INVALID_DEVID;
#endif
#endif

#define NUM_BLOCKS 5
#define BENCH_SIZE (1024 * 1024uL)
static int numBlocks = NUM_BLOCKS;
static word32 bench_size = BENCH_SIZE;

#define BENCH_CIPHER_ADD 0

#define RSA_TEST_BYTES 256 /* 256 bytes * 8 = 2048-bit key length */
#define AES_KEY_SZ 32 /* 32*8 = 256-bit AES KEY */

void check_ret(int val, char* fail)
{
    if (val < 0) {
        printf("%s Failed with error %d\n", fail, val);
    }
    return;
}

int bench_rsa() {
    int    ret;
    size_t bytes;
    WC_RNG rng;
    RsaKey key;
    word32 idx = 0;
    const word32 outSz   = RSA_TEST_BYTES;
    const byte in[] = "Thisismyfakeaeskeythatis32bytes!";
    word32 inLen = XSTRLEN((const char*)in);

    byte tmp[sizeof_client_key_der_2048];
    byte out[RSA_TEST_BYTES];
    byte plain[RSA_TEST_BYTES];

    /* initialize stack structures */
    XMEMSET(&rng, 0, sizeof(rng));
    XMEMSET(&key, 0, sizeof(key));
    XMEMSET(&out, 0, sizeof(out));
    XMEMSET(&plain, 0, sizeof(plain));

    bytes = (size_t)sizeof_client_key_der_2048;

    /* Copy in existing Private RSA key into "tmp" to use for encrypting */
    XMEMCPY(tmp, client_key_der_2048, (size_t)sizeof_client_key_der_2048);

    /* Initialize the RSA key */
    ret = wc_InitRsaKey(&key, HEAP_HINT);
    check_ret(ret, "wc_InitRsaKey_ex");

    /* Decode the private key from buffer "tmp" into RsaKey stucture "key"  */
    ret = wc_RsaPrivateKeyDecode(tmp, &idx, &key, (word32)bytes);
    check_ret(ret, "wc_RsaPrivateKeyDecode");

    /* Initialize the RNG structure */
    ret = wc_InitRng(&rng);
    check_ret(ret, "wc_InitRng");

    /* Implement RSA blinding to defeat side-channel attacks */
    ret = wc_RsaSetRNG(&key, &rng);
    check_ret(ret, "wc_RsaSetRNG");

    /* Sign the AES key effectively "Encrypting it with the private key" */
    ret = wc_RsaSSL_Sign(in, inLen, out, outSz, &key, &rng);
    check_ret(ret, "wc_RsaSSL_Sign");

    return ret;
}


static void bench_aescbc_internal(int useDeviceID, const byte *key,
                                  word32 keySz, const byte *iv,
                                  const char *encLabel, const char *decLabel) {
  int ret = 0, i, count = 0, times, pending = 0;
  WC_DECLARE_ARRAY(enc, Aes, BENCH_MAX_PENDING, sizeof(Aes), HEAP_HINT);
  double start;

  WC_CALLOC_ARRAY(enc, Aes, BENCH_MAX_PENDING, sizeof(Aes), HEAP_HINT);

  /* init keys */
  for (i = 0; i < BENCH_MAX_PENDING; i++) {
    if ((ret = wc_AesInit(enc[i], HEAP_HINT,
                          useDeviceID ? devId : INVALID_DEVID)) != 0) {
      printf("AesInit failed at L%d, ret = %d\n", __LINE__, ret);
      goto exit;
    }

    ret = wc_AesSetKey(enc[i], key, keySz, iv, AES_ENCRYPTION);
    if (ret != 0) {
      printf("AesSetKey failed, ret = %d\n", ret);
      goto exit;
    }
  }

  for (times = 0; times < numBlocks || pending > 0;) {
    /* while free pending slots in queue, submit ops */
    for (i = 0; i < BENCH_MAX_PENDING; i++) {
      ret = wc_AesCbcEncrypt(enc[i], bench_plain, bench_cipher, bench_size);

      if (ret < 0) {
        goto exit_aes_enc;
      } else {
        times++;
      }
    } /* for i */
  }   /* for times */

exit_aes_enc:

  if (ret < 0) {
    goto exit;
  }

#ifdef HAVE_AES_DECRYPT
  /* init keys */
  for (i = 0; i < BENCH_MAX_PENDING; i++) {
    ret = wc_AesSetKey(enc[i], key, keySz, iv, AES_DECRYPTION);
    if (ret != 0) {
      printf("AesSetKey failed, ret = %d\n", ret);
      goto exit;
    }
  }

  for (times = 0; times < numBlocks || pending > 0;) {
    /* while free pending slots in queue, submit ops */
    for (i = 0; i < BENCH_MAX_PENDING; i++) {
      ret = wc_AesCbcDecrypt(enc[i], bench_cipher, bench_plain, bench_size);

      if (ret < 0) {
        goto exit_aes_dec;
      } else {
        times++;
      }
    } /* for i */
  }   /* for times */

exit_aes_dec:

#endif /* HAVE_AES_DECRYPT */

  (void)decLabel;
exit:

  if (WC_ARRAY_OK(enc)) {
    for (i = 0; i < BENCH_MAX_PENDING; i++) {
      wc_AesFree(enc[i]);
    }
    WC_FREE_ARRAY(enc, BENCH_MAX_PENDING, HEAP_HINT);
  }
}

void bench_aescbc(int useDeviceID) {
  bench_aescbc_internal(useDeviceID, bench_key, 32, bench_iv, "AES-256-CBC-enc",
                        "AES-256-CBC-dec");
}

void bench_pbkdf2(void) {
  int ret = 0;
  const char *passwd32 = "passwordpasswordpasswordpassword";
  WOLFSSL_SMALL_STACK_STATIC const byte salt32[] = {
      0x78, 0x57, 0x8E, 0x5a, 0x5d, 0x63, 0xcb, 0x06, 0x78, 0x57, 0x8E,
      0x5a, 0x5d, 0x63, 0xcb, 0x06, 0x78, 0x57, 0x8E, 0x5a, 0x5d, 0x63,
      0xcb, 0x06, 0x78, 0x57, 0x8E, 0x5a, 0x5d, 0x63, 0xcb, 0x06};
  byte derived[32];

  ret = wc_PBKDF2(derived, (const byte *)passwd32, (int)XSTRLEN(passwd32),
                  salt32, (int)sizeof(salt32), 16384, 32, WC_SHA256);
}

typedef enum {
  PBKDF2 = 0,
  AESCBC = 1,
  RSA = 2,
} benchmark_id_t;
benchmark_id_t benchmark_id = 0;

void ecall_benchmark_setup(int id) {
  benchmark_id = (benchmark_id_t)id;
  printf("setup wolfSSL benchmark");

  switch (benchmark_id) {
  case PBKDF2:
    break;
  case AESCBC:
    bench_key = (byte *)bench_key_buf;
    bench_iv = (byte *)bench_iv_buf;
    long bench_buf_size = (int)bench_size + BENCH_CIPHER_ADD;
    if (bench_buf_size % 16)
      bench_buf_size += 16 - (bench_buf_size % 16);
    bench_plain = (byte *)XMALLOC((size_t)bench_buf_size + 16, HEAP_HINT,
                                  DYNAMIC_TYPE_WOLF_BIGINT);
    bench_cipher = (byte *)XMALLOC((size_t)bench_buf_size + 16, HEAP_HINT,
                                   DYNAMIC_TYPE_WOLF_BIGINT);
    break;
  case RSA:
    break;
  }
}

int benchmark_inner() {
  tlblur_enable(30);
  switch (benchmark_id) {
  case PBKDF2:
    bench_pbkdf2();
    break;
  case AESCBC:
    bench_aescbc(1);
    break;
  case RSA:
    return bench_rsa() < 0;
  }
  tlblur_disable();
  return 0;
}
