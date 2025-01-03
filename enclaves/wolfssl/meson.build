subdir('Enclave')

enclaves += {
  'name': 'wolfssl',
  'edl': files('Enclave/encl.edl'),
  'config': files('Enclave/encl.config.xml'),
  'trusted_sources': wolfssl_sources
    + files('Enclave/encl.c')
    + trusted_common_files,
  'untrusted_sources': files('main.c'),
  'include': wolfssl_include,
}
