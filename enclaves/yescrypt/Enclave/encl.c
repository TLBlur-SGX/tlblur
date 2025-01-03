#include "encl_t.h"
#include "tlblur.h"
#include "yescrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"

static void test_PBKDF2_SHA256_raw(const char *passwd, size_t passwdlen,
                                   const char *salt, size_t saltlen, uint64_t c,
                                   size_t dkLen) {
  uint8_t dk[64];
  size_t i;

  // assert(dkLen <= sizeof(dk));

  /* XXX This prints the strings truncated at first NUL */
  // printf("PBKDF2_SHA256(\"%s\", \"%s\", %llu, %llu) =",
  //     passwd, salt, (unsigned long long)c, (unsigned long long)dkLen);

  PBKDF2_SHA256((const uint8_t *)passwd, passwdlen, (const uint8_t *)salt,
                saltlen, c, dk, dkLen);

  // for (i = 0; i < dkLen; i++)
  // 	printf(" %02x", dk[i]);
  // puts("");
}

static void test_PBKDF2_SHA256(const char *passwd, const char *salt, uint64_t c,
                               size_t dkLen) {
  test_PBKDF2_SHA256_raw(passwd, strlen(passwd), salt, strlen(salt), c, dkLen);
}

#include "yescrypt.h"

static void test_yescrypt(const char *passwd, const char *salt,
                          yescrypt_flags_t flags, uint64_t N, uint32_t r,
                          uint32_t p, uint32_t t, uint32_t g, uint32_t dklen) {
  yescrypt_local_t local;
  yescrypt_params_t params = {flags, N, r, p, t, g, 0};
  uint8_t dk[64];
  uint32_t i;

  if (dklen > sizeof(dk) || yescrypt_init_local(&local)) {
    ocall_print_string("FAILED");
    return;
  }

  // printf("yescrypt(\"%s\", \"%s\", %u, %llu, %u, %u, %u, %u) =",
  //     passwd, salt, flags, (unsigned long long)N, r, p, t, g);

  // tlblur_enable(30);
  if (yescrypt_kdf(NULL, &local, (const uint8_t *)passwd, strlen(passwd),
                   (const uint8_t *)salt, strlen(salt), &params, dk, dklen)) {
    yescrypt_free_local(&local);
    ocall_print_string("FAILED");
    return;
  }
  // tlblur_disable();

  yescrypt_free_local(&local);

  // ocall_print_string("ok");
  // for (i = 0; i < dklen; i++)
  // 	printf(" %02x", dk[i]);
  // puts("");
}

uint64_t benchmark_id = 0;

void ecall_select_benchmark(uint64_t id) {
  benchmark_id = id;
}

int benchmark_inner() {
  switch (benchmark_id) {
  case 0:
    test_yescrypt("password", "salt", YESCRYPT_WORM, 16384, 1, 1, 0, 0, 64);
    break;
  case 2:
    test_PBKDF2_SHA256("password", "salt", 16384, 32);
    break;
  }
  // test_PBKDF2_SHA256("password", "salt", 16777216, 32);
  // test_yescrypt("password", "salt", YESCRYPT_WORM, 4096, 1, 1, 0, 0, 64);
  // test_yescrypt("password", "salt", YESCRYPT_WORM, 16384, 1, 1, 0, 0, 64);
  // test_yescrypt("", "", YESCRYPT_WORM, 4, 1, 1, 3, 0, 64);
  // test_PBKDF2_SHA256("passwordPASSWORDpassword",
  //     "saltSALTsaltSALTsaltSALTsaltSALTsalt", 4096, 40);
  // test_scrypt("password", "NaCl", 1024, 8, 16);
  // test_scrypt("pleaseletmein", "SodiumChloride", 16384, 8, 1);
  return 0;
}
