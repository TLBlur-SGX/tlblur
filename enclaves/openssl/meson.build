enclaves += {
  'name': 'openssl',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': files('Enclave/encl.c') + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': include_directories('Enclave'),
  'tlblur_deps': [sgx_pthread, sgx_tcxx, sgx_tsgxssl_crypto_tlblur, sgx_tsgxssl],
  'base_deps': [sgx_pthread, sgx_tcxx, sgx_tsgxssl_crypto, sgx_tsgxssl],
}
